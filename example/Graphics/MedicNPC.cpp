#include "MedicNPC.h"
#include <iostream>

MedicNPC::MedicNPC(double positionX, double positionY, char character, int team , int type)
	: NPC(positionX, positionY, character, team , type)
{
	setCurrentState(new GoToMedicine());
	getCurrentState()->OnEnter(this);
	setMedicine(MEDICINE_MAX / 10);
}

//pTarget
NPC* MedicNPC::getTargetNPC() { return pTarget; }
void MedicNPC::setTargetNPC(NPC* pT) { pTarget = pT; }
//goToTarget
bool MedicNPC::getGoToTarget() { return goToTarget; }
void MedicNPC::setGoToTarget(bool goToTarget) { this->goToTarget = goToTarget; }



bool MedicNPC::getIsGivingMedicine() { return isGivingMedicine; }
void MedicNPC::setIsGivingMedicine(bool isGive) { isGivingMedicine = isGive; }
bool MedicNPC::getIsFillingMedicine() { return isFillingMedicine; }
void MedicNPC::setIsFillingMedicine(bool isFill) { isFillingMedicine = isFill; }

bool MedicNPC::getStayedAtMedicine() { return stayedAtMedicine; }
void MedicNPC::setStayedAtMedicine(bool stayed) { stayedAtMedicine = stayed; }

void MedicNPC::DoSomeWork()
{
	if (isMoving)
	{
		double pos_x, pos_y;
		if (goToTarget)
		{
			getTargetNPC()->getPosition(pos_x, pos_y);
			setTarget(pos_x, pos_y);
			if (medic_counter % 50 == 0)
				PlanPathTo();
			medic_counter++;
		}
		if (FollowPlannedPath(1))//NPC arrive to target if true
		{
			pCurrentState->Transition(this);
		}
	}

	if (isFillingMedicine)
	{
		if (medicine < MEDICINE_MAX )
		{
			medicine += 0.1;
		}
		else if (pTarget)
		{
			if(pTarget->getHp() < MAX_HP && pTarget->getHp() > 0 )
				pCurrentState->Transition(this);
		}
	}

	if (stayedAtMedicine)
	{
		if (pTarget->getHp() < MAX_HP && pTarget->getHp() > 0)
		{
			if (medicine >= MEDICINE_MAX)
				if (auto GoToTargetState = dynamic_cast<GoToTarget*>(pCurrentState))
					GoToTargetState->OnEnter(this);
		}
	}

	if (isGivingMedicine)
	{

		if (goToTarget && pTarget->getHp() < MAX_HP && pTarget->getHp() > 0)
		{
			pTarget->setHp(pTarget->getHp() + 0.5);
			medicine -= 0.1;
			if (medicine <= 0)
				pCurrentState->Transition(this);
		}
		else
		{
			pCurrentState->Transition(this);
		}
	}

}

void MedicNPC::setMedicine(double value) { medicine = value; }

void MedicNPC::show() {
	NPC::show();
	double size = 3.0;

	const double barW = 0.35;
	const double barH = size;
	const double barX0 = x - 0.6;
	const double barY0 = y;

	double na = medicine / MEDICINE_MAX;
	if (na < 0.0) na = 0.0;
	if (na > 1.0) na = 1.0;


	if (na < 0.20)        glColor3d(0.8, 0.1, 0.1);	// dark red  
	else if (na < 0.60)   glColor3d(0.5, 0.8, 0.2);  // yellow-green
	else                  glColor3d(0.0, 0.9, 0.7);  // turquoise

	double fillH = barH * na;
	glBegin(GL_QUADS);
	glVertex2d(barX0, barY0);
	glVertex2d(barX0 + barW, barY0);
	glVertex2d(barX0 + barW, barY0 + fillH);
	glVertex2d(barX0, barY0 + fillH);
	glEnd();
}
