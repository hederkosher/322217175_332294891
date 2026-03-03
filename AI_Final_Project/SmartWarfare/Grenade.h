#pragma once
#include "Bullet.h"

const int NUM_BULLETS = 20;

class Grenade
{
private:
	Bullet* bullets[NUM_BULLETS];
	double x, y;
	double originX, originY;
	double currentX, currentY;
	bool isExploded;
	bool isFlying;
	int fuseTimer;
	double flightProgress;
	int flightDuration;
	int team;

	static const int ARC_POINTS = 40;
	double arcX[ARC_POINTS], arcY[ARC_POINTS];
	int arcCount;
	double arcHeight;
	double perpX, perpY;

	void CalcArcPos(double t, double &outX, double &outY) const;

public:
	Grenade(double srcX, double srcY, double tgtX, double tgtY, int team);
	void Update();
	void Explode(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ]);
	bool getIsExploded() const;
	void setIsExploded(bool value);
	bool getIsFlying() const { return isFlying; }
	int getFuseTimer() const { return fuseTimer; }
	double getTargetX() const { return x; }
	double getTargetY() const { return y; }
	int getTeam() const { return team; }
	void Show();
};
