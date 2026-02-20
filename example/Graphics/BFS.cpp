#include "BFS.h"
#include "Map.h"
#include <queue>
#include <map>
#include <algorithm>

namespace BFS
{
    namespace {

        bool IsObjectType(int i, int j, int object_type) {
            if (i < 0 || i >= MSZ || j < 0 || j >= MSZ) return false;
            return map[i][j] == object_type;
        }

        // Check if adjacent to a stone obstacle (provides cover)
        bool IsAdjacentToStone(int i, int j) {
            for (int di = -2; di <= 2; di++) {
                for (int dj = -2; dj <= 2; dj++) {
                    if (IsObjectType(i + di, j + dj, STONE))
                        return true;
                }
            }
            return false;
        }

        bool IsValidCoverSpot(const std::pair<int, int>& position, int team) {
            if (map[position.first][position.second] != FLOOR)
                return false;
            return IsAdjacentToStone(position.first, position.second);
        }

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
                    if (next.first >= 0 && next.first < MSZ && next.second >= 0 && next.second < MSZ
                        && !visited[next.first][next.second])
                    {
                        if (map[next.first][next.second] == FLOOR ||
                            map[next.first][next.second] == ARMORY ||
                            map[next.first][next.second] == MEDICINE)
                        {
                            visited[next.first][next.second] = true;
                            frontier.push(next);
                            came_from[next] = current;
                        }
                    }
                }
            }
        }

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

    } // end anonymous namespace

    bool FindShortestPathToCover(double startX, double startY, int team, std::vector<std::pair<int, int>>& outPath)
    {
        int start_i = static_cast<int>(startX);
        int start_j = static_cast<int>(startY);

        if (map[start_i][start_j] != FLOOR) return false;

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

        if (goal.first == -1) return false;

        ReconstructPath({ start_i, start_j }, goal, came_from, outPath);
        return true;
    }
}
