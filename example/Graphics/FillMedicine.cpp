#include "FillMedicine.h"

void FillMedicine::OnEnter(NPC* pn)
{
	if (auto mn = dynamic_cast<MedicNPC*>(pn)) {
		mn->setIsFillingMedicine(true);
	}
}

void FillMedicine::Transition(NPC* pn)
{

	OnExit(pn);
	if (auto mn = dynamic_cast<MedicNPC*>(pn)) {
		pn->setCurrentState(new GoToTarget());
		pn->getCurrentState()->OnEnter(pn);
	}
}

void FillMedicine::OnExit(NPC* pn)
{
	if (auto mn = dynamic_cast<MedicNPC*>(pn)) {
		mn->setIsFillingMedicine(false);
	}

}
