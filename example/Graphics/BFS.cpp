#include "BFS.h"
#include "Map.h"
#include <queue>
#include <map>
#include <algorithm>

namespace BFS
{
    namespace {

        // Checks if a cell contains a specific type of object.
        bool IsObjectType(int i, int j, int object_type) {
            // Boundary check before accessing the map
            if (i < 0 || i >= MSZ || j < 0 || j >= MSZ) {
                return false;
            }
            return map[i][j] == object_type;
        }

        // Checks if there is a vertically centered wall of a specific object type.
        bool IsCenteredCoverOfType(int cover_i, int j, int object_type) {
            bool centerIsCorrectType = IsObjectType(cover_i, j, object_type);
            bool aboveIsCorrectType = IsObjectType(cover_i, j - 1, object_type);
            bool belowIsCorrectType = IsObjectType(cover_i, j + 1, object_type);
            return centerIsCorrectType && aboveIsCorrectType && belowIsCorrectType;
        }

        // checks if a position is a valid cover spot
        bool IsValidCoverSpot(const std::pair<int, int>& position, int team) {
            // The destination cell itself must be empty ground
            if (map[position.first][position.second] != 0) {
                return false;
            }
            int stone_offset, tree_offset;
            if (team == 1) { // Team 1 is to the RIGHT of cover
                stone_offset = 0; 
                tree_offset = -4; 
            }
            else { // team == 2 is to the LEFT of cover
                stone_offset = 3;
                tree_offset = 5;
            }

            // Calculate potential cover locations for both types
            int stone_cover_i = position.first + stone_offset;
            int tree_cover_i = position.first + tree_offset;

            bool is_valid_for_stone = IsCenteredCoverOfType(stone_cover_i, position.second, STONE);
            bool is_valid_for_tree = IsCenteredCoverOfType(tree_cover_i, position.second, TREE);
            return is_valid_for_stone || is_valid_for_tree;
        }

        // Explores the neighbors of the current cell
        void ExploreNeighbors(
            const std::pair<int, int>& current,
            std::queue<std::pair<int, int>>& frontier,
            bool visited[MSZ][MSZ],
            std::map<std::pair<int, int>, std::pair<int, int>>& came_from)
        {
            for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
                for (int colOffset = -1; colOffset <= 1; ++colOffset) {
                    if (rowOffset == 0 && colOffset == 0) continue;
                    std::pair<int, int> next = { current.first + rowOffset, current.second + colOffset };
                    if (next.first >= 0 && next.first < MSZ && next.second >= 0 && next.second < MSZ && !visited[next.first][next.second]) {
                        if (map[next.first][next.second] == 0) {
                            visited[next.first][next.second] = true;
                            frontier.push(next);
                            came_from[next] = current;
                        }
                    }
                }
            }
        }

        // Reconstructs the path from the 'came_from' map
        void ReconstructPath(
            const std::pair<int, int>& start,
            const std::pair<int, int>& goal,
            const std::map<std::pair<int, int>, std::pair<int, int>>& came_from,
            std::vector<std::pair<int, int>>& outPath)
        {
            outPath.clear();
            std::pair<int, int> current = goal;
            while (current.first != start.first || current.second != start.second) {
                outPath.push_back(current);
                if (came_from.find(current) == came_from.end()) break;
                current = came_from.at(current);
            }
            outPath.push_back(start);
            std::reverse(outPath.begin(), outPath.end());
        }

    } // end of anonymous namespace

    bool FindShortestPathToCover(double startX, double startY, int team, std::vector<std::pair<int, int>>& outPath)
    {
        int start_i = static_cast<int>(startX);
        int start_j = static_cast<int>(startY);

        if (map[start_i][start_j] != 0) {
            return false;
        }

        std::queue<std::pair<int, int>> frontier;
        frontier.push({ start_i, start_j });

        std::map<std::pair<int, int>, std::pair<int, int>> came_from;
        bool visited[MSZ][MSZ] = { false };
        visited[start_i][start_j] = true;

        std::pair<int, int> goal = { -1, -1 };

        while (!frontier.empty()) {
            std::pair<int, int> current = frontier.front();
            frontier.pop();

            if (IsValidCoverSpot(current, team)) {
                goal = current;
                break;
            }

            ExploreNeighbors(current, frontier, visited, came_from);
        }

        if (goal.first == -1) {
            return false;
        }

        ReconstructPath({ start_i, start_j }, goal, came_from, outPath);
        return true;
    }
}