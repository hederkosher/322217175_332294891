#pragma once
#include "Bullet.h"


const int NUM_BULLETS = 20;


class Grenade
{
private:
	Bullet* bullets[NUM_BULLETS];
	double x, y;
	bool isExploded;
	int team;
public:
	Grenade(double xPos, double yPos, int team);
	void Explode(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ]);
	bool getIsExploded() const;
	void setIsExploded(bool value);
	void Show();
};