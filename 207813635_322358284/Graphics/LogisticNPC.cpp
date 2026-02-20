#include "LogisticNPC.h"
#include <iostream>

LogisticNPC::LogisticNPC(double positionX, double positionY, char character, int team , int type)
	: NPC(positionX, positionY, character, team , type)
{
	setCurrentState(new GoToArmory());
	getCurrentState()->OnEnter(this);
	setAmmo(AMMO_MAX / 10);
}
//warior pointers
void LogisticNPC::setWarriorPointer(WarriorNPC* pW) { pWarrior = pW; }
WarriorNPC* LogisticNPC::getWarriorPointer() { return pWarrior; }

//go to warrior
bool LogisticNPC::getGoToWarrior() { return goToWarrior; }
void LogisticNPC::setGoToWarrior(bool goToW) { goToWarrior = goToW; }

//is giving ammo
bool LogisticNPC::getIsGivingAmmo() { return isGivingAmmo; }
void LogisticNPC::setIsGivingAmmo(bool isGive) { isGivingAmmo = isGive; }
//isFillingAmmo
bool LogisticNPC::getIsFillingAmmo() { return isFillingAmmo; }
void LogisticNPC::setIsFillingAmmo(bool isFill) { isFillingAmmo = isFill; }
void LogisticNPC::setAmmo(double value) { ammo = value; }
//stayed at armory
bool LogisticNPC::getStayedAtArmory() { return stayedAtArmory; }
void LogisticNPC::setStayedAtArmory(bool stayed) { stayedAtArmory = stayed; }

void LogisticNPC::DoSomeWork()
{
	if (isMoving && !isGettingHp)
	{
		double warrior_x, warrior_y;
		if (goToWarrior)
		{
			getWarriorPointer()->getPosition(warrior_x, warrior_y);
			setTarget(warrior_x, warrior_y);
			if(counter % 50 == 0)
				PlanPathTo();
			counter++;
		}

		if(FollowPlannedPath(1))//NPC arrive to target if true
		{
			pCurrentState->Transition(this);
		}
	}

	if (isFillingAmmo)
	{
		if (ammo < AMMO_MAX)
		{
			ammo += 0.1;	
		}
		else if (pWarrior)
		{
			pCurrentState->Transition(this);
		}
	}

	if (stayedAtArmory)
	{
		if (pWarrior->getAmmo() < AMMO_MAX)
		{
			if (ammo >= AMMO_MAX)
				if(auto GoToWarriorState = dynamic_cast<GoToWarrior*>(pCurrentState))
					GoToWarriorState->OnEnter(this);
		}
	}

	if(isGivingAmmo)
	{
		if (pWarrior->getAmmo() < AMMO_MAX)
		{
			pWarrior->setAmmo(pWarrior->getAmmo() + 0.1);
			ammo -= 0.1;
			if (ammo <= 0 )
				pCurrentState->Transition(this);
		}
		else
		{
			pCurrentState->Transition(this);
		}
	}

}

void LogisticNPC::show() {
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