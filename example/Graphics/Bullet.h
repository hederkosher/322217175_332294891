#pragma once
#include "Definitions.h"
#include "NPC.h"

class Bullet {
private:
  double x, y;
  bool isMoving;
  double dirX, dirY;
  int team;
  double damageAmount;  // grenade bullets use higher damage
  double drawRadius;    // 0.5 = normal bullet, 1.0 = grenade particle (larger)

public:
  Bullet(double xPos, double yPos, double angle, int team, double damage = BULLET_DAMAGE, double radius = 0.5);
  void Move(int map[MSZ][MSZ], NPC **team1, NPC **team2,
            double securityMap[MSZ][MSZ]);
  void Show();
  void setIsMoving(bool value) { isMoving = value; }
  bool getIsMoving() const;
  double getX() const { return x; }
  double getY() const { return y; }
};
