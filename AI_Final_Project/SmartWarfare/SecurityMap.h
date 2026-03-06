#pragma once
#include "Definitions.h"

class NPC;

// Global security map (risk field) - used by A* for risk-weighted path cost
extern double securityMap[MSZ][MSZ];

// Security map functions (build & draw)
void CreateSecurityMap();
void DrawSecurityMap();

// Update risk in rooms where combat is active (both teams present). Call each tick before NPC DoSomeWork.
void UpdateSecurityMapForCombatRooms(NPC** team1, NPC** team2);
