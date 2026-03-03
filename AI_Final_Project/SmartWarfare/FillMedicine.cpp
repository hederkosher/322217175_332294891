#include "FillMedicine.h"
#include "NPC.h"
#include "MedicNPC.h"
#include "Map.h"

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
		int idx = mn->getFillingDepotIndex();
		if (idx >= 0 && idx < MAX_DEPOTS) {
			medicineOccupiedBy[idx] = 0;
		}
		mn->setFillingDepotIndex(-1);
		mn->setIsFillingMedicine(false);
	}
}
