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

int armoryX[2], armoryY[2];
int medicineX[2], medicineY[2];

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

// Carve a horizontal corridor segment (3 cells wide)
static void CarveHorizontalCorridor(int x1, int x2, int y) {
  int minX = min(x1, x2);
  int maxX = max(x1, x2);
  for (int i = minX; i <= maxX; i++) {
    for (int w = -1; w <= 1; w++) {
      int j = y + w;
      if (i >= 0 && i < MSZ && j >= 0 && j < MSZ) {
        if (map[i][j] == WALL) {
          map[i][j] = FLOOR;
        }
      }
    }
  }
}

// Carve a vertical corridor segment (3 cells wide)
static void CarveVerticalCorridor(int x, int y1, int y2) {
  int minY = min(y1, y2);
  int maxY = max(y1, y2);
  for (int j = minY; j <= maxY; j++) {
    for (int w = -1; w <= 1; w++) {
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

// Place cover obstacles (stones) inside a room
static void PlaceObstaclesInRoom(const Room &room) {
  int roomW = room.x2 - room.x1 + 1;
  int roomH = room.y2 - room.y1 + 1;
  if (roomW < 10 || roomH < 10)
    return;

  int numObstacles = 2 + rand() % 3; // 2-4 per room
  for (int o = 0; o < numObstacles; o++) {
    int ox = room.x1 + 3 + rand() % (roomW - 7);
    int oy = room.y1 + 3 + rand() % (roomH - 7);

    bool blocked = false;
    for (int i = 0; i < 3 && !blocked; i++)
      for (int j = 0; j < 3 && !blocked; j++)
        if (map[ox + i][oy + j] != FLOOR)
          blocked = true;

    if (!blocked) {
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
          map[ox + i][oy + j] = STONE;
    }
  }
}

// Place armory and medicine depots randomly in rooms
static void PlaceDepots() {
  int usedRooms[4] = {-1, -1, -1, -1};
  int usedCount = 0;

  // Place 2 armory depots
  for (int d = 0; d < 2; d++) {
    int r;
    int attempts = 0;
    do {
      r = rand() % numRooms;
      attempts++;
    } while (attempts < 100 && (r == usedRooms[0] || r == usedRooms[1] ||
                                r == usedRooms[2] || r == usedRooms[3]));

    usedRooms[usedCount++] = r;

    int roomW = rooms[r].x2 - rooms[r].x1 + 1;
    int roomH = rooms[r].y2 - rooms[r].y1 + 1;
    int ox = rooms[r].x1 + 3 + rand() % max(1, roomW - 8);
    int oy = rooms[r].y1 + 3 + rand() % max(1, roomH - 8);

    armoryX[d] = ox;
    armoryY[d] = oy;

    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (ox + i < MSZ && oy + j < MSZ)
          map[ox + i][oy + j] = ARMORY;
  }

  // Place 2 medicine depots
  for (int d = 0; d < 2; d++) {
    int r;
    int attempts = 0;
    do {
      r = rand() % numRooms;
      attempts++;
    } while (attempts < 100 && (r == usedRooms[0] || r == usedRooms[1] ||
                                r == usedRooms[2] || r == usedRooms[3]));

    usedRooms[usedCount++] = r;

    int roomW = rooms[r].x2 - rooms[r].x1 + 1;
    int roomH = rooms[r].y2 - rooms[r].y1 + 1;
    int ox = rooms[r].x1 + 3 + rand() % max(1, roomW - 8);
    int oy = rooms[r].y1 + 3 + rand() % max(1, roomH - 8);

    medicineX[d] = ox;
    medicineY[d] = oy;

    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (ox + i < MSZ && oy + j < MSZ)
          map[ox + i][oy + j] = MEDICINE;
  }
}

// Find a walkable spawn position within a room
static bool FindSpawnInRoom(const Room &room, double &outX, double &outY) {
  for (int attempt = 0; attempt < 50; attempt++) {
    int rx = room.x1 + 2 + rand() % max(1, room.x2 - room.x1 - 5);
    int ry = room.y1 + 2 + rand() % max(1, room.y2 - room.y1 - 5);

    bool clear = true;
    for (int i = 0; i < 3 && clear; i++)
      for (int j = 0; j < 3 && clear; j++)
        if (rx + i >= MSZ || ry + j >= MSZ || map[rx + i][ry + j] != FLOOR)
          clear = false;

    if (clear) {
      outX = rx;
      outY = ry;
      return true;
    }
  }
  outX = room.centerX();
  outY = room.centerY();
  return false;
}

// Place troops in rooms
static void PlaceTroops(NPC **team1, NPC **team2) {
  // Team 1 spawns in a right-side room (column 2)
  int rightRooms[] = {2, 5, 8};
  int r1 = rightRooms[rand() % 3];
  if (r1 >= numRooms)
    r1 = numRooms - 1;

  // Team 2 spawns in a left-side room (column 0)
  int leftRooms[] = {0, 3, 6};
  int r2 = leftRooms[rand() % 3];
  if (r2 >= numRooms)
    r2 = 0;

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

  // Team 2
  FindSpawnInRoom(rooms[r2], sx, sy);
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

// Generate the maze with rooms and corridors
static void GenerateMaze() {
  // Fill entire map with walls
  for (int i = 0; i < MSZ; i++)
    for (int j = 0; j < MSZ; j++) {
      map[i][j] = WALL;
      roomId[i][j] = 0;
    }

  numRooms = 0;
  int sectorW = MSZ / GRID_COLS;
  int sectorH = MSZ / GRID_ROWS;

  // Create rooms in grid pattern
  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      int sectorX = col * sectorW;
      int sectorY = row * sectorH;

      int roomW = 14 + rand() % 8; // 14-21 cells
      int roomH = 14 + rand() % 8;

      if (roomW > sectorW - 4)
        roomW = sectorW - 4;
      if (roomH > sectorH - 4)
        roomH = sectorH - 4;

      int marginX = max(0, sectorW - roomW - 4);
      int marginY = max(0, sectorH - roomH - 4);

      int roomX = sectorX + 2 + (marginX > 0 ? rand() % marginX : 0);
      int roomY = sectorY + 2 + (marginY > 0 ? rand() % marginY : 0);

      // Clamp to map bounds
      if (roomX + roomW >= MSZ)
        roomW = MSZ - roomX - 1;
      if (roomY + roomH >= MSZ)
        roomH = MSZ - roomY - 1;

      int id = numRooms + 1;
      rooms[numRooms] = {id, roomX, roomY, roomX + roomW - 1,
                         roomY + roomH - 1};

      for (int i = roomX; i < roomX + roomW; i++)
        for (int j = roomY; j < roomY + roomH; j++) {
          map[i][j] = FLOOR;
          roomId[i][j] = id;
        }

      numRooms++;
    }
  }

  // Connect adjacent rooms with corridors
  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      int idx = row * GRID_COLS + col;
      if (col < GRID_COLS - 1) {
        int rightIdx = row * GRID_COLS + (col + 1);
        ConnectRooms(rooms[idx], rooms[rightIdx]);
      }
      if (row < GRID_ROWS - 1) {
        int bottomIdx = (row + 1) * GRID_COLS + col;
        ConnectRooms(rooms[idx], rooms[bottomIdx]);
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
          glColor3d(0.85, 0.85, 0.75); // room floor
        else
          glColor3d(0.7, 0.7, 0.65); // passage floor
        break;
      case WALL:
        glColor3d(0.2, 0.2, 0.25);
        break;
      case STONE:
        glColor3d(0.45, 0.42, 0.38);
        break;
      case ARMORY:
        glColor3d(0.9, 0.75, 0.1); // gold for armory
        break;
      case MEDICINE:
        glColor3d(0.9, 0.2, 0.3); // red/pink for medicine
        break;
      default:
        glColor3d(0.2, 0.2, 0.25);
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

  // Draw room outlines for clarity
  glColor3d(0.3, 0.3, 0.35);
  glLineWidth(1.0f);
  for (int r = 0; r < numRooms; r++) {
    glBegin(GL_LINE_LOOP);
    glVertex2d(rooms[r].x1, rooms[r].y1);
    glVertex2d(rooms[r].x2 + 1, rooms[r].y1);
    glVertex2d(rooms[r].x2 + 1, rooms[r].y2 + 1);
    glVertex2d(rooms[r].x1, rooms[r].y2 + 1);
    glEnd();
  }

  // Draw 'A' on armories and '+' on medicine depots
  glColor3d(0.0, 0.0, 0.0);
  for (int d = 0; d < 2; d++) {
    glRasterPos2d(armoryX[d] + 0.8, armoryY[d] + 0.8);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'A');

    glRasterPos2d(medicineX[d] + 0.8, medicineY[d] + 0.8);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '+');
  }
}
