#include "GiveAmmo.h"
#include "GoToArmory.h"
#include "NPC.h"
#include "SupplyNPC.h"


void GiveAmmo::OnEnter(NPC *pn) {
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    ln->setIsGivingAmmo(true);
  }
}

void GiveAmmo::Transition(NPC *pn) {
  OnExit(pn);
  pn->setCurrentState(new GoToArmory());
  pn->getCurrentState()->OnEnter(pn);
}

void GiveAmmo::OnExit(NPC *pn) {
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    ln->setIsGivingAmmo(false);
    ln->setGoToWarrior(false);
  }
}
