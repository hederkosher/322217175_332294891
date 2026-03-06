#include "Map.h"
#include "MedicNPC.h"
#include "NPC.h"
#include "SupplyNPC.h"
#include "WarriorNPC.h"
#include "glut.h"
#include <algorithm>
#include <iostream>
#include <math.h>
#include <stdlib.h>

using namespace std;

int map[MSZ][MSZ] = {0};
Room rooms[MAX_ROOMS];
int numRooms = 0;
int roomId[MSZ][MSZ] = {0};

int numArmories = 0;
int numMedicine = 0;
int armoryX[MAX_DEPOTS], armoryY[MAX_DEPOTS];
int medicineX[MAX_DEPOTS], medicineY[MAX_DEPOTS];

int armoryOccupiedBy[MAX_DEPOTS] = {};
int medicineOccupiedBy[MAX_DEPOTS] = {};

char npcOccupancyGrid[MSZ][MSZ] = {};

void UpdateNpcOccupancy(NPC **team1, NPC **team2) {
  for (int i = 0; i < MSZ; i++)
    for (int j = 0; j < MSZ; j++)
      npcOccupancyGrid[i][j] = 0;
  for (int t = 0; t < 2; t++) {
    NPC **team = (t == 0) ? team1 : team2;
    if (!team) continue;
    char idBase = (t == 0) ? 1 : 5;
    for (int k = 0; k < TEAM_SIZE; k++) {
      if (!team[k] || team[k]->getHp() <= 0) continue;
      double px, py;
      team[k]->getPosition(px, py);
      int fi = (int)px, fj = (int)py;
      for (int a = 0; a < 3; a++)
        for (int b = 0; b < 3; b++) {
          int ni = fi + a, nj = fj + b;
          if (ni >= 0 && ni < MSZ && nj >= 0 && nj < MSZ)
            npcOccupancyGrid[ni][nj] = idBase + k;
        }
    }
  }
}

void ReleaseAllDepots() {
  for (int i = 0; i < MAX_DEPOTS; i++) {
    armoryOccupiedBy[i] = 0;
    medicineOccupiedBy[i] = 0;
  }
}

bool HasLineOfSight(double x1, double y1, double x2, double y2) {
  int x0 = (int)(x1 + 1.5);
  int y0 = (int)(y1 + 1.5);
  int xEnd = (int)(x2 + 1.5);
  int yEnd = (int)(y2 + 1.5);

  int dx = abs(xEnd - x0);
  int dy = -abs(yEnd - y0);
  int sx = (x0 < xEnd) ? 1 : -1;
  int sy = (y0 < yEnd) ? 1 : -1;
  int err = dx + dy;

  while (true) {
    if (x0 < 0 || x0 >= MSZ || y0 < 0 || y0 >= MSZ)
      return false;
    if (map[x0][y0] == WALL || map[x0][y0] == STONE)
      return false;
    if (x0 == xEnd && y0 == yEnd)
      break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
  return true;
}

bool RoomHasEnemies(int rId, NPC **enemyTeam) {
  if (!enemyTeam || rId <= 0) return false;
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (enemyTeam[i] && enemyTeam[i]->getHp() > 0) {
      if (enemyTeam[i]->getCurrentRoom() == rId)
        return true;
    }
  }
  return false;
}

int GetRoomAt(double x, double y) {
  int ix = (int)x;
  int iy = (int)y;
  if (ix >= 0 && ix < MSZ && iy >= 0 && iy < MSZ)
    return roomId[ix][iy];
  return 0;
}

bool AreInSameRoom(double x1, double y1, double x2, double y2) {
  int r1 = GetRoomAt(x1, y1);
  int r2 = GetRoomAt(x2, y2);
  return r1 > 0 && r1 == r2;
}

Room *GetRoomById(int id) {
  for (int i = 0; i < numRooms; i++)
    if (rooms[i].id == id)
      return &rooms[i];
  return nullptr;
}

// Carve a horizontal corridor segment (5 cells wide - allows 3x3 NPCs to pass)
static void CarveHorizontalCorridor(int x1, int x2, int y) {
  int minX = min(x1, x2);
  int maxX = max(x1, x2);
  for (int i = minX; i <= maxX; i++) {
    for (int w = -2; w <= 2; w++) {
      int j = y + w;
      if (i >= 0 && i < MSZ && j >= 0 && j < MSZ) {
        if (map[i][j] == WALL) {
          map[i][j] = FLOOR;
        }
      }
    }
  }
}

// Carve a vertical corridor segment (5 cells wide - allows 3x3 NPCs to pass)
static void CarveVerticalCorridor(int x, int y1, int y2) {
  int minY = min(y1, y2);
  int maxY = max(y1, y2);
  for (int j = minY; j <= maxY; j++) {
    for (int w = -2; w <= 2; w++) {
      int i = x + w;
      if (i >= 0 && i < MSZ && j >= 0 && j < MSZ) {
        if (map[i][j] == WALL) {
          map[i][j] = FLOOR;
        }
      }
    }
  }
}

// Connect two rooms with an L-shaped corridor
static void ConnectRooms(const Room &a, const Room &b) {
  int cx1 = a.centerX();
  int cy1 = a.centerY();
  int cx2 = b.centerX();
  int cy2 = b.centerY();

  CarveHorizontalCorridor(cx1, cx2, cy1);
  CarveVerticalCorridor(cx2, cy1, cy2);
}

static const int MIN_OBJ_DIST = 7;

// Place cover obstacles (stones) inside a room with minimum spacing
static void PlaceObstaclesInRoom(const Room &room) {
  int roomW = room.x2 - room.x1 + 1;
  int roomH = room.y2 - room.y1 + 1;
  if (roomW < 14 || roomH < 14)
    return;

  int maxObs = (roomW >= 18 && roomH >= 18) ? 3 : 2;
  int numObstacles = 1 + rand() % maxObs;

  int placedX[8], placedY[8];
  int placedCount = 0;
  int margin = 5;

  for (int o = 0; o < numObstacles; o++) {
    for (int attempt = 0; attempt < 40; attempt++) {
      int ox = room.x1 + margin + rand() % max(1, roomW - margin * 2 - 2);
      int oy = room.y1 + margin + rand() % max(1, roomH - margin * 2 - 2);

      bool blocked = false;
      for (int i = 0; i < 3 && !blocked; i++)
        for (int j = 0; j < 3 && !blocked; j++)
          if (ox + i >= MSZ || oy + j >= MSZ || map[ox + i][oy + j] != FLOOR)
            blocked = true;

      if (!blocked) {
        bool tooClose = false;
        for (int p = 0; p < placedCount && !tooClose; p++) {
          int dx = ox - placedX[p];
          int dy = oy - placedY[p];
          if (dx * dx + dy * dy < MIN_OBJ_DIST * MIN_OBJ_DIST)
            tooClose = true;
        }
        if (tooClose) continue;

        for (int i = 0; i < 3; i++)
          for (int j = 0; j < 3; j++)
            map[ox + i][oy + j] = STONE;
        placedX[placedCount] = ox;
        placedY[placedCount] = oy;
        placedCount++;
        break;
      }
    }
  }
}

// Check that a 3x3 area at (ox, oy) is all FLOOR and has clearance from STONE
static bool IsDepotSpotClear(int ox, int oy) {
  for (int i = -2; i < 5; i++)
    for (int j = -2; j < 5; j++) {
      int ci = ox + i, cj = oy + j;
      if (ci < 0 || ci >= MSZ || cj < 0 || cj >= MSZ) continue;
      if (i >= 0 && i < 3 && j >= 0 && j < 3) {
        if (map[ci][cj] != FLOOR) return false;
      } else {
        if (map[ci][cj] == STONE) return false;
      }
    }
  return true;
}

// Place armory and medicine depots randomly in rooms
static void PlaceDepots() {
  numArmories = 2;
  numMedicine = 2;
  int usedRooms[MAX_DEPOTS * 2];
  for (int i = 0; i < MAX_DEPOTS * 2; i++) usedRooms[i] = -1;
  int usedCount = 0;

  for (int d = 0; d < numArmories; d++) {
    int r;
    int attempts = 0;
    do {
      r = rand() % numRooms;
      attempts++;
      bool used = false;
      for (int i = 0; i < usedCount; i++) if (usedRooms[i] == r) { used = true; break; }
      if (used) r = -1;
    } while (attempts < 100 && r < 0);
    if (r < 0) r = 0;
    usedRooms[usedCount++] = r;
    int roomW = rooms[r].x2 - rooms[r].x1 + 1;
    int roomH = rooms[r].y2 - rooms[r].y1 + 1;
    int ox = 0, oy = 0;
    for (int a = 0; a < 60; a++) {
      ox = rooms[r].x1 + 4 + rand() % max(1, roomW - 10);
      oy = rooms[r].y1 + 4 + rand() % max(1, roomH - 10);
      if (IsDepotSpotClear(ox, oy)) break;
    }
    armoryX[d] = ox;
    armoryY[d] = oy;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (ox + i < MSZ && oy + j < MSZ) map[ox + i][oy + j] = ARMORY;
  }

  for (int d = 0; d < numMedicine; d++) {
    int r;
    int attempts = 0;
    do {
      r = rand() % numRooms;
      attempts++;
      bool used = false;
      for (int i = 0; i < usedCount; i++) if (usedRooms[i] == r) { used = true; break; }
      if (used) r = -1;
    } while (attempts < 100 && r < 0);
    if (r < 0) r = 0;
    usedRooms[usedCount++] = r;
    int roomW = rooms[r].x2 - rooms[r].x1 + 1;
    int roomH = rooms[r].y2 - rooms[r].y1 + 1;
    int ox = 0, oy = 0;
    for (int a = 0; a < 60; a++) {
      ox = rooms[r].x1 + 4 + rand() % max(1, roomW - 10);
      oy = rooms[r].y1 + 4 + rand() % max(1, roomH - 10);
      if (IsDepotSpotClear(ox, oy)) break;
    }
    medicineX[d] = ox;
    medicineY[d] = oy;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (ox + i < MSZ && oy + j < MSZ) map[ox + i][oy + j] = MEDICINE;
  }
}

static const int SPAWN_CLEARANCE = 4;

static bool HasNearbyObstacle(int rx, int ry) {
  for (int di = -SPAWN_CLEARANCE; di <= SPAWN_CLEARANCE + 2; di++) {
    for (int dj = -SPAWN_CLEARANCE; dj <= SPAWN_CLEARANCE + 2; dj++) {
      int ci = rx + di;
      int cj = ry + dj;
      if (ci >= 0 && ci < MSZ && cj >= 0 && cj < MSZ) {
        if (map[ci][cj] == STONE || map[ci][cj] == ARMORY || map[ci][cj] == MEDICINE)
          return true;
      }
    }
  }
  return false;
}

static double spawnedX[8];
static double spawnedY[8];
static int spawnedCount = 0;
static const int MIN_SPAWN_DIST = 5;

static bool IsPositionBlocked(int cx, int cy) {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      int ci = cx + i, cj = cy + j;
      if (ci < 0 || ci >= MSZ || cj < 0 || cj >= MSZ) return true;
      if (map[ci][cj] != FLOOR) return true;
    }
  return false;
}

// Find a walkable spawn position within a room, away from obstacles and other spawns
static bool FindSpawnInRoom(const Room &room, double &outX, double &outY) {
  int margin = 4;
  for (int attempt = 0; attempt < 80; attempt++) {
    int rx = room.x1 + margin + rand() % max(1, room.x2 - room.x1 - margin * 2 - 2);
    int ry = room.y1 + margin + rand() % max(1, room.y2 - room.y1 - margin * 2 - 2);

    bool clear = true;
    for (int i = 0; i < 3 && clear; i++)
      for (int j = 0; j < 3 && clear; j++)
        if (rx + i >= MSZ || ry + j >= MSZ || map[rx + i][ry + j] != FLOOR)
          clear = false;
    if (!clear) continue;

    if (HasNearbyObstacle(rx, ry)) continue;

    bool tooCloseToOther = false;
    for (int s = 0; s < spawnedCount && !tooCloseToOther; s++) {
      double dx = rx - spawnedX[s];
      double dy = ry - spawnedY[s];
      if (dx * dx + dy * dy < MIN_SPAWN_DIST * MIN_SPAWN_DIST)
        tooCloseToOther = true;
    }
    if (tooCloseToOther) continue;

    outX = rx;
    outY = ry;
    spawnedX[spawnedCount] = rx;
    spawnedY[spawnedCount] = ry;
    spawnedCount++;
    return true;
  }

  // Fallback: brute-force scan for ANY walkable 3x3 spot (never use blocked center)
  for (int rx = room.x1 + 2; rx <= room.x2 - 2; rx++) {
    for (int ry = room.y1 + 2; ry <= room.y2 - 2; ry++) {
      if (IsPositionBlocked(rx, ry)) continue;
      bool tooClose = false;
      for (int s = 0; s < spawnedCount && !tooClose; s++) {
        double dx = rx - spawnedX[s], dy = ry - spawnedY[s];
        if (dx * dx + dy * dy < MIN_SPAWN_DIST * MIN_SPAWN_DIST) tooClose = true;
      }
      if (tooClose) continue;
      outX = rx;
      outY = ry;
      spawnedX[spawnedCount] = rx;
      spawnedY[spawnedCount] = ry;
      spawnedCount++;
      return true;
    }
  }

  // Last resort: any 3x3 FLOOR, ignore spacing
  for (int rx = room.x1 + 2; rx <= room.x2 - 2; rx++) {
    for (int ry = room.y1 + 2; ry <= room.y2 - 2; ry++) {
      if (!IsPositionBlocked(rx, ry)) {
        outX = rx;
        outY = ry;
        spawnedX[spawnedCount] = rx;
        spawnedY[spawnedCount] = ry;
        spawnedCount++;
        return true;
      }
    }
  }
  outX = room.centerX();
  outY = room.centerY();
  return false;
}

static const int MIN_SPAWN_ROOM = 16;

// Place troops in rooms (each team gets a large room far apart)
static void PlaceTroops(NPC **team1, NPC **team2) {
  if (numRooms < 2) return;
  spawnedCount = 0;

  // Collect rooms large enough for team spawning
  int bigRooms[MAX_ROOMS];
  int bigCount = 0;
  for (int i = 0; i < numRooms; i++) {
    int w = rooms[i].x2 - rooms[i].x1 + 1;
    int h = rooms[i].y2 - rooms[i].y1 + 1;
    if (w >= MIN_SPAWN_ROOM && h >= MIN_SPAWN_ROOM)
      bigRooms[bigCount++] = i;
  }
  if (bigCount < 2) {
    bigCount = numRooms;
    for (int i = 0; i < numRooms; i++) bigRooms[i] = i;
  }

  int r1 = bigRooms[rand() % bigCount];
  int r2 = bigRooms[rand() % bigCount];
  int tries = 0;
  while (r2 == r1 && bigCount > 1 && tries < 50) {
    r2 = bigRooms[rand() % bigCount];
    tries++;
  }

  double sx, sy;

  // Team 1
  FindSpawnInRoom(rooms[r1], sx, sy);
  team1[0] = new WarriorNPC(sx, sy, 'W', 1, WARRIOR_1_1);
  FindSpawnInRoom(rooms[r1], sx, sy);
  team1[1] = new WarriorNPC(sx, sy, 'W', 1, WARRIOR_1_2);
  FindSpawnInRoom(rooms[r1], sx, sy);
  team1[2] = new MedicNPC(sx, sy, 'M', 1, MEDIC_1);
  FindSpawnInRoom(rooms[r1], sx, sy);
  team1[3] = new SupplyNPC(sx, sy, 'P', 1, SUPPLY_1);

  // Team 2 (reset spawn tracker for different room)
  spawnedCount = 0;
  FindSpawnInRoom(rooms[r2], sx, sy);
  team2[0] = new WarriorNPC(sx, sy, 'W', 2, WARRIOR_2_1);
  FindSpawnInRoom(rooms[r2], sx, sy);
  team2[1] = new WarriorNPC(sx, sy, 'W', 2, WARRIOR_2_2);
  FindSpawnInRoom(rooms[r2], sx, sy);
  team2[2] = new MedicNPC(sx, sy, 'M', 2, MEDIC_2);
  FindSpawnInRoom(rooms[r2], sx, sy);
  team2[3] = new SupplyNPC(sx, sy, 'P', 2, SUPPLY_2);
  team2[0] = new WarriorNPC(sx, sy, 'W', 2, WARRIOR_2_1);
  FindSpawnInRoom(rooms[r2], sx, sy);
  team2[1] = new WarriorNPC(sx, sy, 'W', 2, WARRIOR_2_2);
  FindSpawnInRoom(rooms[r2], sx, sy);
  team2[2] = new MedicNPC(sx, sy, 'M', 2, MEDIC_2);
  FindSpawnInRoom(rooms[r2], sx, sy);
  team2[3] = new SupplyNPC(sx, sy, 'P', 2, SUPPLY_2);

  // Set team references so NPCs can find teammates/enemies
  for (int i = 0; i < TEAM_SIZE; i++) {
    team1[i]->setMyTeam(team1);
    team1[i]->setEnemyTeam(team2);
    team2[i]->setMyTeam(team2);
    team2[i]->setEnemyTeam(team1);
  }
}

// Generate the maze with rooms and corridors (random grid size = random room count)
static void GenerateMaze() {
  for (int i = 0; i < MSZ; i++)
    for (int j = 0; j < MSZ; j++) {
      map[i][j] = WALL;
      roomId[i][j] = 0;
    }

  numRooms = 0;
  int gridRows = 2 + rand() % 3;  // 2-4 rows
  int gridCols = 2 + rand() % 3;  // 2-4 cols -> 4 to 16 rooms
  int sectorW = MSZ / max(1, gridCols);
  int sectorH = MSZ / max(1, gridRows);

  for (int row = 0; row < gridRows; row++) {
    for (int col = 0; col < gridCols; col++) {
      int sectorX = col * sectorW;
      int sectorY = row * sectorH;

      int roomW = 14 + rand() % 8;
      int roomH = 14 + rand() % 8;
      if (roomW > sectorW - 4) roomW = sectorW - 4;
      if (roomH > sectorH - 4) roomH = sectorH - 4;
      roomW = max(14, roomW);
      roomH = max(14, roomH);

      int marginX = max(0, sectorW - roomW - 4);
      int marginY = max(0, sectorH - roomH - 4);
      int roomX = sectorX + 2 + (marginX > 0 ? rand() % marginX : 0);
      int roomY = sectorY + 2 + (marginY > 0 ? rand() % marginY : 0);

      if (roomX + roomW >= MSZ) roomW = MSZ - roomX - 1;
      if (roomY + roomH >= MSZ) roomH = MSZ - roomY - 1;
      if (roomW < 14 || roomH < 14) continue;

      int id = numRooms + 1;
      rooms[numRooms] = {id, roomX, roomY, roomX + roomW - 1, roomY + roomH - 1};
      for (int i = roomX; i < roomX + roomW; i++)
        for (int j = roomY; j < roomY + roomH; j++) {
          map[i][j] = FLOOR;
          roomId[i][j] = id;
        }
      numRooms++;
    }
  }

  for (int row = 0; row < gridRows; row++) {
    for (int col = 0; col < gridCols; col++) {
      int idx = row * gridCols + col;
      if (idx >= numRooms) break;
      if (col < gridCols - 1) {
        int rightIdx = row * gridCols + (col + 1);
        if (rightIdx < numRooms) ConnectRooms(rooms[idx], rooms[rightIdx]);
      }
      if (row < gridRows - 1) {
        int bottomIdx = (row + 1) * gridCols + col;
        if (bottomIdx < numRooms) ConnectRooms(rooms[idx], rooms[bottomIdx]);
      }
    }
  }
}

void InitMap(NPC **team1, NPC **team2) {
  GenerateMaze();

  // Place cover obstacles in each room
  for (int r = 0; r < numRooms; r++)
    PlaceObstaclesInRoom(rooms[r]);

  PlaceDepots();
  PlaceTroops(team1, team2);
}

void DrawMap() {
  for (int i = 0; i < MSZ; i++) {
    for (int j = 0; j < MSZ; j++) {
      switch (map[i][j]) {
      case FLOOR:
        if (roomId[i][j] > 0)
          glColor3d(0.04, 0.04, 0.10);
        else
          glColor3d(0.02, 0.02, 0.06);
        break;
      case WALL:
        glColor3d(0.06, 0.06, 0.14);
        break;
      case STONE:
        glColor3d(0.12, 0.08, 0.18);
        break;
      case ARMORY:
        glColor3d(0.35, 0.22, 0.0);
        break;
      case MEDICINE:
        glColor3d(0.85, 0.85, 0.88);
        break;
      default:
        glColor3d(0.06, 0.06, 0.14);
        break;
      }

      glBegin(GL_POLYGON);
      glVertex2d(i, j);
      glVertex2d(i, j + 1);
      glVertex2d(i + 1, j + 1);
      glVertex2d(i + 1, j);
      glEnd();
    }
  }

  // Neon room outlines
  glColor3d(0.0, 0.7, 1.0);
  glLineWidth(1.5f);
  for (int r = 0; r < numRooms; r++) {
    glBegin(GL_LINE_LOOP);
    glVertex2d(rooms[r].x1, rooms[r].y1);
    glVertex2d(rooms[r].x2 + 1, rooms[r].y1);
    glVertex2d(rooms[r].x2 + 1, rooms[r].y2 + 1);
    glVertex2d(rooms[r].x1, rooms[r].y2 + 1);
    glEnd();
  }
  glLineWidth(1.0f);

  // Ammo pack: row of bullet cartridges
  for (int d = 0; d < numArmories; d++) {
    double ax = armoryX[d];
    double ay = armoryY[d];
    int numBullets = 4;
    double bw = 0.55;
    double gap = (3.0 - numBullets * bw) / (numBullets + 1);
    double bh = 2.4;
    double by0 = ay + 0.3;

    for (int b = 0; b < numBullets; b++) {
      double bx = ax + gap + b * (bw + gap);
      double tipH = bh * 0.22;
      double caseH = bh * 0.50;
      double baseH = bh * 0.28;

      // Bullet tip (bright yellow, pointed)
      glColor3d(1.0, 0.9, 0.15);
      glBegin(GL_TRIANGLES);
      glVertex2d(bx + bw * 0.5, by0 + bh);
      glVertex2d(bx + 0.02, by0 + bh - tipH);
      glVertex2d(bx + bw - 0.02, by0 + bh - tipH);
      glEnd();

      // Upper casing (golden orange)
      glColor3d(0.85, 0.6, 0.08);
      glBegin(GL_QUADS);
      glVertex2d(bx, by0 + baseH);
      glVertex2d(bx + bw, by0 + baseH);
      glVertex2d(bx + bw, by0 + bh - tipH);
      glVertex2d(bx, by0 + bh - tipH);
      glEnd();

      // Lower casing / base (dark brown)
      glColor3d(0.45, 0.32, 0.08);
      glBegin(GL_QUADS);
      glVertex2d(bx - 0.04, by0);
      glVertex2d(bx + bw + 0.04, by0);
      glVertex2d(bx + bw + 0.04, by0 + baseH);
      glVertex2d(bx - 0.04, by0 + baseH);
      glEnd();

      // Primer ring (darker line at very bottom)
      glColor3d(0.3, 0.2, 0.05);
      glBegin(GL_QUADS);
      glVertex2d(bx - 0.04, by0);
      glVertex2d(bx + bw + 0.04, by0);
      glVertex2d(bx + bw + 0.04, by0 + baseH * 0.3);
      glVertex2d(bx - 0.04, by0 + baseH * 0.3);
      glEnd();

      // Casing highlight (vertical shine)
      glColor3d(1.0, 0.82, 0.3);
      glBegin(GL_QUADS);
      glVertex2d(bx + bw * 0.35, by0 + baseH);
      glVertex2d(bx + bw * 0.5, by0 + baseH);
      glVertex2d(bx + bw * 0.5, by0 + bh - tipH);
      glVertex2d(bx + bw * 0.35, by0 + bh - tipH);
      glEnd();
    }
  }
  // Health pack: white box with red cross
  for (int d = 0; d < numMedicine; d++) {
    double mx = medicineX[d];
    double my = medicineY[d];
    double sz = 3.0;

    // Outer border (light grey)
    glColor3d(0.7, 0.72, 0.75);
    glBegin(GL_QUADS);
    glVertex2d(mx - 0.1, my - 0.1);
    glVertex2d(mx + sz + 0.1, my - 0.1);
    glVertex2d(mx + sz + 0.1, my + sz + 0.1);
    glVertex2d(mx - 0.1, my + sz + 0.1);
    glEnd();

    // White background
    glColor3d(0.95, 0.95, 0.97);
    glBegin(GL_QUADS);
    glVertex2d(mx + 0.15, my + 0.15);
    glVertex2d(mx + sz - 0.15, my + 0.15);
    glVertex2d(mx + sz - 0.15, my + sz - 0.15);
    glVertex2d(mx + 0.15, my + sz - 0.15);
    glEnd();

    // Red cross
    double cx = mx + sz * 0.5;
    double cy = my + sz * 0.5;
    double arm = 0.95;
    double thick = 0.38;

    // Cross shadow (dark red, offset slightly)
    glColor3d(0.45, 0.05, 0.1);
    glBegin(GL_QUADS);
    glVertex2d(cx - thick + 0.08, cy - arm - 0.08);
    glVertex2d(cx + thick + 0.08, cy - arm - 0.08);
    glVertex2d(cx + thick + 0.08, cy + arm - 0.08);
    glVertex2d(cx - thick + 0.08, cy + arm - 0.08);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2d(cx - arm + 0.08, cy - thick - 0.08);
    glVertex2d(cx + arm + 0.08, cy - thick - 0.08);
    glVertex2d(cx + arm + 0.08, cy + thick - 0.08);
    glVertex2d(cx - arm + 0.08, cy + thick - 0.08);
    glEnd();

    // Cross body (red)
    glColor3d(0.78, 0.08, 0.12);
    glBegin(GL_QUADS);
    glVertex2d(cx - thick, cy - arm);
    glVertex2d(cx + thick, cy - arm);
    glVertex2d(cx + thick, cy + arm);
    glVertex2d(cx - thick, cy + arm);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2d(cx - arm, cy - thick);
    glVertex2d(cx + arm, cy - thick);
    glVertex2d(cx + arm, cy + thick);
    glVertex2d(cx - arm, cy + thick);
    glEnd();
  }
}
