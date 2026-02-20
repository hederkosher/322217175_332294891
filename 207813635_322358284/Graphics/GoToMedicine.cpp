#include "GoToMedicine.h"

void GoToMedicine::OnEnter(NPC* pn)
{
	pn->setIsMoving(true);
	if (pn->getTeam() == 1)
		pn->setTarget(MEDICINE_1_X, MEDICINE_1_Y);
	else
		pn->setTarget(MEDICINE_2_X, MEDICINE_2_Y);
	pn->PlanPathTo();//AStar
}

void GoToMedicine::Transition(NPC* pn)
{
	OnExit(pn);

	pn->setCurrentState(new FillMedicine());
	pn->getCurrentState()->OnEnter(pn);
}

void GoToMedicine::OnExit(NPC* pn)
{
	pn->setIsMoving(false);

}
