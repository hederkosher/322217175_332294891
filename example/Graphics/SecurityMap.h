#pragma once
#include "Definitions.h"

// Global security map (risk field)
extern double securityMap[MSZ][MSZ];

// Security map functions (build & draw)
void CreateSecurityMap();
void DrawSecurityMap();
