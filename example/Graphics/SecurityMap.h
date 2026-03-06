#pragma once
#include "Definitions.h"

class NPC;

// Global security map (risk field)
extern double securityMap[MSZ][MSZ];

// Security map functions (build & draw)
void CreateSecurityMap();
void DrawSecurityMap();

// Update security map from NPC positions in active combat rooms (with decay)
void UpdateSecurityMap(NPC** team1, NPC** team2);
