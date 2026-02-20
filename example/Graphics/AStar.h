#pragma once
#include "Definitions.h"
#include <utility>
#include <vector>


struct AStarNode {
  int i, j;
  double g;   // starting cost
  double h;   // heuristic cost
  int pi, pj; // parent path
};

bool FindPath(int si, int sj, // start (grid)
              int ti, int tj, // target (grid)
              std::vector<std::pair<int, int>> &outPath);
