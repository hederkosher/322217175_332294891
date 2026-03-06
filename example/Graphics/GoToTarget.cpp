#include "GoToInjuredState.h"
#include "GiveMedicine.h"
#include "MedicNPC.h"
#include "NPC.h"


void GoToInjuredState::OnEnter(NPC *pn) {
  double x, y;
  if (auto mn = dynamic_cast<MedicNPC *>(pn)) {
    pn->setIsMoving(true);
    if (mn->getTargetNPC() && mn->getTargetNPC()->getHp() < MAX_HP / 2 &&
        mn->getTargetNPC()->getHp() > 0) {
      mn->setGoToInjured(true);
      mn->getTargetNPC()->getPosition(x, y);
      pn->setTarget(x, y);
      mn->setWaitingAtMedicine(false);
      mn->PlanPathTo();
      return;
    }
    mn->setWaitingAtMedicine(true);
  }
}

void GoToInjuredState::Transition(NPC *pn) {
  OnExit(pn);
  if (auto mn = dynamic_cast<MedicNPC *>(pn)) {
    pn->setCurrentState(new GiveMedicine());
    pn->getCurrentState()->OnEnter(pn);
    return;
  }
}

void GoToInjuredState::OnExit(NPC *pn) { pn->setIsMoving(false); }
