#include "WarriorNPC.h"
#include "Map.h"
#include "Bullet.h"
#include <iostream>
#include <math.h>
#include "MoveToTargetState.h"
#include "AttackState.h"

extern int map[MSZ][MSZ];
extern const int TREE_W;
extern const int TREE_H;


WarriorNPC::WarriorNPC(double positionX, double positionY, char character, int team , int type)
	: NPC(positionX, positionY, character, team , type)
{
	hp = MAX_HP;
	ammo = AMMO_MAX;
	pathIndex = -1;
    isAttacking = false;
    setCurrentState(new MoveToTargetState());
    getCurrentState()->OnEnter(this);
}

bool WarriorNPC::isInRisk() const
{
	bool lowHp = hp < MAX_HP / 2.0;
	bool lowAmmo = ammo < AMMO_MAX / 3.0; 
	return lowHp || lowAmmo;
}


void WarriorNPC::setAmmo(double value) { ammo = value; }
double WarriorNPC::getAmmo() { return ammo; }
void WarriorNPC::setIsAttacking(bool value) { isAttacking = value; }


bool WarriorNPC::FindVisibleEnemy(double& outX, double& outY)
{
    for (int i = 0; i < MSZ; i++) {
        for (int j = 0; j < MSZ; j++) {

            if (visibilityMap[i][j] > 0) {
                outX = i; 
                outY = j;
                return true;
            }
        }
    }
    return false; 
}

static int warrior_counter_grenade_frames = 0;

void WarriorNPC::DoSomeWork()
{
    if (pCurrentState)
        pCurrentState->Transition(this);

    if (isMoving && !isGettingHp)
    {
        arrivedAtTarget = false;
        if (FollowPlannedPath(0.15))
        {
            arrivedAtTarget = true;
			std::cout << "Warrior NPC #" << team << " type: " << npcType << " reached target (" << targetX << ", " << targetY << ")" << std::endl;
       }
    }

	if (!isMoving && !isGettingHp && x==targetX && y == targetY)
	{
        int const RADIUS = 30;
        int offsetX = (rand() % (2 * RADIUS + 1)) - RADIUS;
        int offsetY = (rand() % (2 * RADIUS + 1)) - RADIUS;

        int targetI = x + offsetX;
        int targetJ = y + offsetY;
        setTarget(targetI, targetJ);
        PlanPathTo();
        if (auto idleState = dynamic_cast<IdleState*>(getCurrentState())) {
            //attack is in idle state, order to go to attack
            if (getCurrentState() != nullptr) {
                getCurrentState()->OnExit(this);
            }
            setCurrentState(new AttackState());
            getCurrentState()->OnEnter(this);
        }
	}

    else if (isAttacking)
    {
        arrivedAtTarget = false;
        double enemyX, enemyY;
        if (FindVisibleEnemy(enemyX, enemyY) && ammo > 0)
        {
            setAmmo(ammo - 0.01);

			//calculate angle to enemy
            double dx = enemyX  - (x + 1.5); //enemy center
            double dy = enemyY  - (y + 1.5);
			double angle = atan2(dy, dx); //radians

			//creare bullet if not already created
            if (pBullet == nullptr)
            {
				pBullet = new Bullet(x + 1.5 , y + 1.5 , angle, team);
				pBullet->setIsMoving(true);
            }
        }
    }
    if (auto idleState = dynamic_cast<IdleState*>(getCurrentState())) {
        if (warrior_counter_grenade_frames % 180 ==0)
        {
			int offset = team == 1 ? -5 : 5;
            double grenadeX = x + offset;
            double grenadeY = y + offset;
            double angle = (rand() % 360) * 3.14 / 180; // random angle
            double distance = 20.0; // fixed distance
            double targetX = grenadeX + distance * cos(angle);
            double targetY = grenadeY + distance * sin(angle);
            // Clamp target within map boundaries
            if (targetX < 0) targetX = 0;
            if (targetX > MSZ) targetX = MSZ;
            if (targetY < 0) targetY = 0;
            if (targetY > MSZ) targetY = MSZ;
            // Create and throw grenade
            if (pGrenade == nullptr && ammo >=5)
            {
                pGrenade = new Grenade(grenadeX, grenadeY, team);
                std::cout << GRENADE << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << RESET << std::endl;
				std::cout << GRENADE << "Warrior NPC #" << team << " type: " << npcType << " threw a grenade!" << RESET << std::endl;
                std::cout << GRENADE << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << RESET << std::endl;
                pGrenade->setIsExploded(true);
				ammo = ammo - 5;
            }
		}
        warrior_counter_grenade_frames++;
    }
}

void WarriorNPC::show() {
	NPC::show();
	double size = 3.0;

	const double barW = 0.35;     // bar width
	const double barH = size;     // bar height
	const double barX0 = x - 0.6; // left of NPC
	const double barY0 = y;

	// Normalize ammo to [0,1]
	double na = ammo / AMMO_MAX;
	if (na < 0.0) na = 0.0;
	if (na > 1.0) na = 1.0;

	// Discrete colors:
	// 0–20%   -> Red
	// 20–60%  -> Orange
	// 60–100% -> Purple
	if (na < 0.20)        glColor3d(1.0, 0.0, 0.0);   // red
	else if (na < 0.60)   glColor3d(0.0, 0.0, 0.6);   // dark blue
	else                  glColor3d(0.6, 0.0, 0.8);   // purple

	// Fill (bottom -> up)
	double fillH = barH * na;
	glBegin(GL_QUADS);
	glVertex2d(barX0, barY0);
	glVertex2d(barX0 + barW, barY0);
	glVertex2d(barX0 + barW, barY0 + fillH);
	glVertex2d(barX0, barY0 + fillH);
	glEnd();
}

Bullet* WarriorNPC::getBullet() const { return pBullet; }

void WarriorNPC::setBullet(Bullet* bullet) { pBullet = bullet; }

Grenade* WarriorNPC::getGrenade() const { return pGrenade; }

void WarriorNPC::setGrenade(Grenade* grenade) { pGrenade = grenade; }

