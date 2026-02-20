#pragma once
#include <vector>
#include <utility>

namespace BFS
{
    /**
     * @brief Finds the shortest path to a valid cover position based on team.
     * Team 1 seeks cover to their LEFT (destination is to the RIGHT of the cover).
     * Team 2 seeks cover to their RIGHT (destination is to the LEFT of the cover).
     * @param startX The starting X coordinate.
     * @param startY The starting Y coordinate.
     * @param team The team of the NPC (1 or 2).
     * @param outPath A vector to be filled with the coordinates of the path.
     * @return True if a path was found, false otherwise.
     */
    bool FindShortestPathToCover(double startX, double startY, int team, std::vector<std::pair<int, int>>& outPath);
}