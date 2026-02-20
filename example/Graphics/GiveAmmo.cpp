#include "GiveAmmo.h"
#include "NPC.h"
#include "LogisticNPC.h"
#include "GoToArmory.h"

void GiveAmmo::OnEnter(NPC* pn)
{
	if (auto ln = dynamic_cast<LogisticNPC*>(pn)) {
		ln->setIsGivingAmmo(true);
	}
}

void GiveAmmo::Transition(NPC* pn)
{
	OnExit(pn);
	pn->setCurrentState(new GoToArmory());
	pn->getCurrentState()->OnEnter(pn);
}

void GiveAmmo::OnExit(NPC* pn)
{
	if (auto ln = dynamic_cast<LogisticNPC*>(pn)) {
		ln->setIsGivingAmmo(false);
		ln->setGoToWarrior(false);
	}
}
