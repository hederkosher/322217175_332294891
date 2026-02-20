#pragma once
#include "glut.h"
#include <math.h>
#include "Definitions.h"
#include "State.h"
#include <vector>
#include <utility>


extern const double SPEED;
extern const int MAX_HP;

class NPC
{
protected:
	int team; // 1 or 2
	char symbol;
	double x, y;
	double targetX, targetY;
	double directionX, directionY;
	bool isMoving;
	bool isGettingHp = false;
	double hp;
	State* pCurrentState = nullptr;
	std::vector<std::pair<int, int>> path;
	int pathIndex = -1;

	//visibility map
	int npcType; 
	int visibilityMap[MSZ][MSZ];
	double viewDistance;

public:
	NPC(double positionX, double positionY, char character, int team , int type);
	virtual bool isInRisk() const;
	virtual void DoSomeWork() = 0;
	double Distance(double x1, double y1, double x2, double y2);
	virtual void show();
	void setIsMoving(bool value);
	void setTarget(double tx, double ty);
	void setMovingDirection();
	int getTeam();
	void setCurrentState(State* s);
	State* getCurrentState();
	void getPosition(double& px, double& py);
	double getHp();
	void setHp(double value);
	int getNpcType();

	//getters setter for is getting hp
	bool getIsGettingHp();
	void setIsGettingHp(bool value);

	//visibility map getter
	int (*getVisibilityMap())[MSZ];

	//AStar
	bool PlanPathTo();
	bool FollowPlannedPath(double minDist);
	void DrawPath() const;

	//visibility map functions
	bool HasLineOfSight(double targetX, double targetY) const;
	virtual void UpdateVisibility(NPC* myTeam[TEAM_SIZE] , NPC* enemyTeam[TEAM_SIZE]);
	virtual void DrawVisibilityMap() const;


	//BFS
	void setPath(const std::vector<std::pair<int, int>>& newPath);
	bool getIsMoving() const;
	int getPathIndex() const;

};

