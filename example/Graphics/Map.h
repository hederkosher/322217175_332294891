#pragma once
#include "Definitions.h"

class NPC;

// Terrain map
extern int map[MSZ][MSZ];

// Room system
struct Room {
  int id;
  int x1, y1, x2, y2; // interior bounds (inclusive)
  int centerX() const { return (x1 + x2) / 2; }
  int centerY() const { return (y1 + y2) / 2; }
};

extern Room rooms[MAX_ROOMS];
extern int numRooms;
extern int roomId[MSZ][MSZ]; // 0 = wall/passage, >0 = room id

// Depot positions (counts and positions are random at runtime)
extern int numArmories;
extern int numMedicine;
extern int armoryX[MAX_DEPOTS], armoryY[MAX_DEPOTS];
extern int medicineX[MAX_DEPOTS], medicineY[MAX_DEPOTS];

// Line-of-sight: true if no WALL or STONE between (x1,y1) and (x2,y2)
bool HasLineOfSight(double x1, double y1, double x2, double y2);

// True if any living enemy is in this room (for medics/supply to avoid fights)
bool RoomHasEnemies(int roomId, NPC **enemyTeam);

// Room utility functions
int GetRoomAt(double x, double y);
bool AreInSameRoom(double x1, double y1, double x2, double y2);
Room *GetRoomById(int id);

void InitMap(NPC **team1, NPC **team2);
void DrawMap();
