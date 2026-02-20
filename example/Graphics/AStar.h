#pragma once
#include <vector>
#include <utility>

#include "Definitions.h"
#include "Map.h"
#include "SecurityMap.h"

struct AStarNode {
    int i, j;
    double g;   //stating cost
	double h;   //huristic cost
    int pi, pj; //parent path
};

bool FindPath(
    int si, int sj,          // start (grid)
    int ti, int tj,          // target (grid)
    std::vector<std::pair<int, int>>& outPath 
);
