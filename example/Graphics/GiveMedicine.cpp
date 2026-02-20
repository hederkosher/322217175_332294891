#include "GiveMedicine.h"

void GiveMedicine::OnEnter(NPC* pn)
{
	if (auto mn = dynamic_cast<MedicNPC*>(pn)) {
		mn->setIsGivingMedicine(true);
		mn->getTargetNPC()->setIsGettingHp(true);
	}
}

void GiveMedicine::Transition(NPC* pn)
{

	OnExit(pn);
	pn->setCurrentState(new GoToMedicine());
	pn->getCurrentState()->OnEnter(pn);
	
}

void GiveMedicine::OnExit(NPC* pn)
{
	if (auto mn = dynamic_cast<MedicNPC*>(pn)) {
		mn->setIsGivingMedicine(false);
		mn->setGoToTarget(false);
		mn->getTargetNPC()->setIsGettingHp(false);
	}

}
