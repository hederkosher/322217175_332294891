#include "FillAmmo.h"
#include "NPC.h"
#include "SupplyNPC.h"

void FillAmmo::OnEnter(NPC *pn) {
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    ln->setIsFillingAmmo(true);
  }
}

void FillAmmo::Transition(NPC *pn) {
  OnExit(pn);
  pn->setCurrentState(new GoToWarrior());
  pn->getCurrentState()->OnEnter(pn);
}

void FillAmmo::OnExit(NPC *pn) {
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    ln->setIsFillingAmmo(false);
  }
}
