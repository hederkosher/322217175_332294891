#include "FillAmmo.h"
#include "NPC.h"
#include "LogisticNPC.h"

void FillAmmo::OnEnter(NPC* pn)
{
	if (auto ln = dynamic_cast<LogisticNPC*>(pn)) {
		ln->setIsFillingAmmo(true);
	}
}

void FillAmmo::Transition(NPC* pn)
{
	OnExit(pn);
	pn->setCurrentState(new GoToWarrior());
	pn->getCurrentState()->OnEnter(pn);
}

void FillAmmo::OnExit(NPC* pn)
{
	if (auto ln = dynamic_cast<LogisticNPC*>(pn)) {
		ln->setIsFillingAmmo(false);
	}
}
