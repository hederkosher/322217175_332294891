#pragma once
#include "Definitions.h"
#include "State.h"
#include "glut.h"
#include <math.h>
#include <utility>
#include <vector>

class NPC {
protected:
  int team; // 1 or 2
  char symbol;
  double x, y;
  double targetX, targetY;
  double directionX, directionY;
  bool isMoving;
  bool isGettingHp = false;
  double hp;
  State *pCurrentState = nullptr;
  std::vector<std::pair<int, int>> path;
  int pathIndex = -1;

  int npcType;
  int visibilityMap[MSZ][MSZ];
  double viewDistance;

  // Team references
  NPC **myTeam = nullptr;
  NPC **enemyTeam = nullptr;

  // Personality traits (randomly generated)
  double aggressiveness; // 0.3 - 0.9: higher = more aggressive
  double cautiousness;   // 0.3 - 0.9: higher = flees earlier

public:
  NPC(double positionX, double positionY, char character, int team, int type);
  virtual bool isInRisk() const;
  virtual void DoSomeWork() = 0;
  double Distance(double x1, double y1, double x2, double y2);
  virtual void show();
  void setIsMoving(bool value);
  void setTarget(double tx, double ty);
  void setMovingDirection();
  int getTeam();
  void setCurrentState(State *s);
  State *getCurrentState();
  void getPosition(double &px, double &py);
  double getHp();
  void setHp(double value);
  int getNpcType();

  bool getIsGettingHp();
  void setIsGettingHp(bool value);

  int (*getVisibilityMap())[MSZ];

  // A* pathfinding
  bool PlanPathTo();
  bool FollowPlannedPath(double minDist);
  void DrawPath() const;

  // Visibility
  bool HasLineOfSight(double targetX, double targetY) const;
  virtual void UpdateVisibility(NPC *myTeamArr[TEAM_SIZE],
                                NPC *enemyTeamArr[TEAM_SIZE]);
  virtual void DrawVisibilityMap() const;

  // BFS
  void setPath(const std::vector<std::pair<int, int>> &newPath);
  bool getIsMoving() const;
  int getPathIndex() const;

  // Team references
  void setMyTeam(NPC **team) { myTeam = team; }
  NPC **getMyTeam() { return myTeam; }
  void setEnemyTeam(NPC **team) { enemyTeam = team; }
  NPC **getEnemyTeam() { return enemyTeam; }

  // Personality getters
  double getAggressiveness() const { return aggressiveness; }
  double getCautiousness() const { return cautiousness; }

  // Room awareness
  int getCurrentRoom() const;
};
