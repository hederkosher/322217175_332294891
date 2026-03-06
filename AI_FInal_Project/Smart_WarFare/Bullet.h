#pragma once
#include "Definitions.h"
#include "NPC.h"

class Bullet {
public:
  static const int TRAIL_LEN = 20;

private:
  double x, y;
  bool isMoving;
  double dirX, dirY;
  int team;
  double damage;

  // Trail ring buffer
  double trailX[TRAIL_LEN];
  double trailY[TRAIL_LEN];
  int trailIdx  = 0;
  int trailCount = 0;

public:
  Bullet(double xPos, double yPos, double angle, int team, double damage);
  void Move(int map[MSZ][MSZ], NPC **team1, NPC **team2,
            double securityMap[MSZ][MSZ]);
  void Move(int map[MSZ][MSZ], NPC **team1, NPC **team2,
            double securityMap[MSZ][MSZ],
            bool hitTeam1[TEAM_SIZE], bool hitTeam2[TEAM_SIZE]);
  void Show();
  void setIsMoving(bool value) { isMoving = value; }
  bool getIsMoving() const;
  double getX() const { return x; }
  double getY() const { return y; }
};
