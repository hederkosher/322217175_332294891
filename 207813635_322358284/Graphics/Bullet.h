#pragma once
#include "Definitions.h"
#include "NPC.h"


extern const double SPEED;
extern const double SECURITY;

class Bullet
{
private:
	double x, y;
	bool isMoving;
	double dirX, dirY;
	int team;
public:
	Bullet(double xPos, double yPos, double angle, int team);
	void Move(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ]);
	void Show();
	void setIsMoving(bool value) { isMoving = value; }
	bool getIsMoving() const;
	double getX() const { return x; }
	double getY() const { return y; }
};
