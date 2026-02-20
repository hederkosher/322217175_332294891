#include "GoToArmory.h"

void GoToArmory::OnEnter(NPC* pn)
{
	pn->setIsMoving(true);
	if (pn->getTeam() == 1)
		pn->setTarget(ARMORY_1_X, ARMORY_1_Y);
	else
		pn->setTarget(ARMORY_2_X, ARMORY_2_Y);
	pn->PlanPathTo();//AStar
}

void GoToArmory::Transition(NPC* pn)
{
	OnExit(pn);
	pn->setCurrentState(new FillAmmo());
	pn->getCurrentState()->OnEnter(pn);
}

void GoToArmory::OnExit(NPC* pn)
{
	pn->setIsMoving(false);

}
