#include "AStar.h"
#include "NPC.h"
#include <queue>
#include <cmath>
#include <limits>

extern int map[MSZ][MSZ];
extern double securityMap[MSZ][MSZ];

static bool isCellOccupiedByOtherNPC(int i, int j, NPC **team1, NPC **team2,
                                     NPC *exclude, int ti, int tj) {
  for (int t = 0; t < 2; t++) {
    NPC **team = (t == 0) ? team1 : team2;
    if (!team) continue;
    for (int k = 0; k < TEAM_SIZE; k++) {
      NPC *n = team[k];
      if (!n || n == exclude || n->getHp() <= 0) continue;
      double px, py;
      n->getPosition(px, py);
      int fi = (int)px, fj = (int)py;
      if (i >= fi && i < fi + 3 && j >= fj && j < fj + 3) {
        if (ti >= fi && ti < fi + 3 && tj >= fj && tj < fj + 3)
          continue;
        return true;
      }
    }
  }
  return false;
}

static inline bool inBounds(int i, int j) {
    return i >= 0 && i < MSZ && j >= 0 && j < MSZ;
}

static inline bool isBlocked(int cell) {
    return (cell == WALL) || (cell == STONE);
}

static inline bool cellBlockedOrOOB(int i, int j) {
    if (!inBounds(i, j)) return true;
    return isBlocked(map[i][j]);
}

static const double RISK_WEIGHT = 10.0;
static const int NPC_HALF = 1;

static const int DIRS = 8;
static const int di[DIRS] = { -1,-1,-1, 0, 0, 1, 1, 1 };
static const int dj[DIRS] = { -1, 0, 1,-1, 1,-1, 0, 1 };

static inline double stepCost(int k) {
    return (di[k] == 0 || dj[k] == 0) ? 1.0 : std::sqrt(2.0);
}

static inline double heuristic(int i, int j, int ti, int tj) {
    double dx = double(ti - i), dy = double(tj - j);
    return std::sqrt(dx * dx + dy * dy);
}

static inline bool footprintFree(int i, int j) {
    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            int ni = i + a;
            int nj = j + b;
            if (cellBlockedOrOOB(ni, nj))
                return false;
        }
    }
    return true;
}

static inline bool canStepDiagWithFootprint(int ci, int cj, int ni, int nj) {
    int sdi = ni - ci, sdj = nj - cj;
    if (sdi != 0 && sdj != 0) {
        if (!footprintFree(ci, cj + sdj)) return false;
        if (!footprintFree(ci + sdi, cj)) return false;
    }
    return true;
}

bool FindPath(int si, int sj, int ti, int tj, std::vector<std::pair<int, int>>& outPath) {
    outPath.clear();
    if (!inBounds(si, sj) || !inBounds(ti, tj)) return false;

    if (!footprintFree(si, sj) || !footprintFree(ti, tj)) return false;

    static double gScore[MSZ][MSZ];
    static double fScore[MSZ][MSZ];
    static int    pi[MSZ][MSZ], pj[MSZ][MSZ];
    static bool   closed[MSZ][MSZ];

    for (int i = 0; i < MSZ; ++i) {
        for (int j = 0; j < MSZ; ++j) {
            gScore[i][j] = std::numeric_limits<double>::infinity();
            fScore[i][j] = std::numeric_limits<double>::infinity();
            pi[i][j] = pj[i][j] = -1;
            closed[i][j] = false;
        }
    }

    struct QItem { int i, j; double f; };
    struct Cmp { bool operator()(const QItem& a, const QItem& b) const { return a.f > b.f; } };
    std::priority_queue<QItem, std::vector<QItem>, Cmp> open;

    gScore[si][sj] = 0.0;
    fScore[si][sj] = heuristic(si, sj, ti, tj);
    open.push(QItem{ si, sj, fScore[si][sj] });

    static const int MAX_ASTAR_EXPANSIONS = 1200;  // cap cost so FPS stays high in late match
    int expansions = 0;
    while (!open.empty()) {
        if (expansions++ >= MAX_ASTAR_EXPANSIONS) return false;
        QItem top = open.top();
        open.pop();
        int ci = top.i;
        int cj = top.j;

        if (closed[ci][cj]) continue;
        closed[ci][cj] = true;

        if (ci == ti && cj == tj) {
            int qi = ti, qj = tj;
            std::vector<std::pair<int, int>> rev;
            while (qi != -1 && qj != -1) {
                rev.push_back(std::make_pair(qi, qj));
                int nqi = pi[qi][qj], nqj = pj[qi][qj];
                qi = nqi; qj = nqj;
            }
            outPath.assign(rev.rbegin(), rev.rend());
            return true;
        }

        for (int k = 0; k < DIRS; ++k) {
            int ni = ci + di[k], nj = cj + dj[k];
            if (!inBounds(ni, nj)) continue;

            if (!footprintFree(ni, nj)) continue;

            if (!canStepDiagWithFootprint(ci, cj, ni, nj)) continue;

            double base = stepCost(k);
            double risk = 0.5 * (securityMap[ci][cj] + securityMap[ni][nj]);
            double cost = base * (1.0 + RISK_WEIGHT * risk);

            double tentative = gScore[ci][cj] + cost;
            if (tentative < gScore[ni][nj]) {
                gScore[ni][nj] = tentative;
                fScore[ni][nj] = tentative + heuristic(ni, nj, ti, tj);
                pi[ni][nj] = ci; pj[ni][nj] = cj;
                open.push(QItem{ ni, nj, fScore[ni][nj] });
            }
        }
    }
    return false;
}

bool FindPath(int si, int sj, int ti, int tj,
              std::vector<std::pair<int, int>>& outPath,
              NPC **team1, NPC **team2, NPC *exclude) {
  outPath.clear();
  if (!inBounds(si, sj) || !inBounds(ti, tj)) return false;

  auto footprintFreeWithNPC = [&](int i, int j) {
    for (int a = 0; a < 3; ++a) {
      for (int b = 0; b < 3; ++b) {
        int ni = i + a, nj = j + b;
        if (cellBlockedOrOOB(ni, nj)) return false;
        if (team1 || team2) {
          if (isCellOccupiedByOtherNPC(ni, nj, team1, team2, exclude, ti, tj))
            return false;
        }
      }
    }
    return true;
  };

  if (!footprintFreeWithNPC(si, sj) || !footprintFreeWithNPC(ti, tj))
    return false;

  static double gScore2[MSZ][MSZ];
  static double fScore2[MSZ][MSZ];
  static int pi2[MSZ][MSZ], pj2[MSZ][MSZ];
  static bool closed2[MSZ][MSZ];

  for (int i = 0; i < MSZ; ++i) {
    for (int j = 0; j < MSZ; ++j) {
      gScore2[i][j] = std::numeric_limits<double>::infinity();
      fScore2[i][j] = std::numeric_limits<double>::infinity();
      pi2[i][j] = pj2[i][j] = -1;
      closed2[i][j] = false;
    }
  }

  struct QItem { int i, j; double f; };
  struct Cmp { bool operator()(const QItem& a, const QItem& b) const { return a.f > b.f; } };
  std::priority_queue<QItem, std::vector<QItem>, Cmp> open;

  gScore2[si][sj] = 0.0;
  fScore2[si][sj] = heuristic(si, sj, ti, tj);
  open.push(QItem{ si, sj, fScore2[si][sj] });

  static const int MAX_ASTAR_EXPANSIONS = 1200;  // cap cost so FPS stays high in late match
  int expansions = 0;
  while (!open.empty()) {
    if (expansions++ >= MAX_ASTAR_EXPANSIONS) return false;
    QItem top = open.top();
    open.pop();
    int ci = top.i, cj = top.j;

    if (closed2[ci][cj]) continue;
    closed2[ci][cj] = true;

    if (ci == ti && cj == tj) {
      std::vector<std::pair<int, int>> rev;
      int qi = ti, qj = tj;
      while (qi != -1 && qj != -1) {
        rev.push_back({ qi, qj });
        int nqi = pi2[qi][qj], nqj = pj2[qi][qj];
        qi = nqi; qj = nqj;
      }
      outPath.assign(rev.rbegin(), rev.rend());
      return true;
    }

    for (int k = 0; k < DIRS; ++k) {
      int ni = ci + di[k], nj = cj + dj[k];
      if (!inBounds(ni, nj)) continue;
      if (!footprintFreeWithNPC(ni, nj)) continue;
      int sdi = ni - ci, sdj = nj - cj;
      if (sdi != 0 && sdj != 0) {
        if (!footprintFreeWithNPC(ci, cj + sdj)) continue;
        if (!footprintFreeWithNPC(ci + sdi, cj)) continue;
      }

      double base = stepCost(k);
      double risk = 0.5 * (securityMap[ci][cj] + securityMap[ni][nj]);
      double cost = base * (1.0 + RISK_WEIGHT * risk);
      double tentative = gScore2[ci][cj] + cost;
      if (tentative < gScore2[ni][nj]) {
        gScore2[ni][nj] = tentative;
        fScore2[ni][nj] = tentative + heuristic(ni, nj, ti, tj);
        pi2[ni][nj] = ci; pj2[ni][nj] = cj;
        open.push(QItem{ ni, nj, fScore2[ni][nj] });
      }
    }
  }
  return false;
}
