#pragma once
#include "Bullet.h"

const int NUM_BULLETS = 20;
const int GRENADE_FUSE_MS = 1500;

class Grenade
{
private:
	Bullet* bullets[NUM_BULLETS];
	double x, y;
	double startX, startY, targetX, targetY;
	int spawnTimeMs;
	bool isExploded;
	int team;
	void createFragments();
public:
	Grenade(double startX, double startY, double targetX, double targetY, int team);
	void Update();
	void Explode(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ]);
	bool getIsExploded() const;
	void setIsExploded(bool value);
	void Show();
	~Grenade();
};