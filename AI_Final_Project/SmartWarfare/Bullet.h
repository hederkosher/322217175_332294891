#pragma once
#include "Definitions.h"
#include "NPC.h"

class Bullet {
private:
  double x, y;
  bool isMoving;
  double dirX, dirY;
  int team;
  double damageAmount;
  double drawRadius;
  bool pierceEnemies;  // Grenade particles pierce through enemies

  static const int TRAIL_LEN = 40;
  double trailX[TRAIL_LEN], trailY[TRAIL_LEN];
  int trailIdx;
  int trailCount;

public:
  Bullet(double xPos, double yPos, double angle, int team, double damage = BULLET_DAMAGE, double radius = 0.5, bool pierce = false);
  void Move(int map[MSZ][MSZ], NPC **team1, NPC **team2,
            double securityMap[MSZ][MSZ]);
  void Show();
  void setIsMoving(bool value) { isMoving = value; }
  bool getIsMoving() const;
  double getX() const { return x; }
  double getY() const { return y; }
  int getTeam() const { return team; }
};
