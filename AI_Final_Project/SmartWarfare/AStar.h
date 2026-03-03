#pragma once
#include "Definitions.h"
#include <utility>
#include <vector>

class NPC;

struct AStarNode {
  int i, j;
  double g;   // starting cost
  double h;   // heuristic cost
  int pi, pj; // parent path
};

bool FindPath(int si, int sj, // start (grid)
              int ti, int tj, // target (grid)
              std::vector<std::pair<int, int>> &outPath);

// With NPC avoidance - treats other NPCs as obstacles so units don't block each other at entrances
bool FindPath(int si, int sj, int ti, int tj,
              std::vector<std::pair<int, int>> &outPath,
              NPC **team1, NPC **team2, NPC *exclude);
