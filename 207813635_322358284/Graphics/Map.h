#pragma once
#include "Definitions.h"
#include "NPC.h"
#include "LogisticNPC.h"
#include "MedicNPC.h"
#include "WarriorNPC.h"
#include "CommanderNPC.h"

// Terrain map
extern int map[MSZ][MSZ];

//  Trees (3x4) 
extern const int TREE;
extern const int TREE_W;      // 3
extern const int TREE_H;      // 4
extern const int TREE_COUNT;  // 10
extern int treeX[];
extern int treeY[];

static void placeStones();
static void placeWater();
void generateSquare(int type, NPC** npc, char symbol, int team, int x_min, int x_max, int y_min, int y_max, NPC** teamNPC);
void generateSquareMapObject(int type, int x, int y);
static void placeArmoryMedicne();
static void PlaceTroops(NPC** team1, NPC** team2);
static void placeTrees();
void InitMap(NPC** team1, NPC** team2);
void drawChar(int i, int j, int type, char character);
void DrawMap();