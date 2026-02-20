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

// Depot positions (2 armories, 2 medicine depots)
extern int armoryX[2], armoryY[2];
extern int medicineX[2], medicineY[2];

// Room utility functions
int GetRoomAt(double x, double y);
bool AreInSameRoom(double x1, double y1, double x2, double y2);
Room *GetRoomById(int id);

void InitMap(NPC **team1, NPC **team2);
void DrawMap();
