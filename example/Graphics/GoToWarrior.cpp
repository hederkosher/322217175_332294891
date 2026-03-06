#include "GoToNeedyWarriorState.h"
#include "GiveAmmo.h"
#include "NPC.h"
#include "SupplyNPC.h"


void GoToNeedyWarriorState::OnEnter(NPC *pn) {
  double x, y;
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    pn->setIsMoving(true);
    if (ln->getWarriorPointer() && ln->getWarriorPointer()->getHp() > 0 &&
        ln->getWarriorPointer()->getAmmo() < AMMO_MAX / 2) {
      ln->setGoToNeedyWarrior(true);
      ln->getWarriorPointer()->getPosition(x, y);
      pn->setTarget(x, y);
      ln->setWaitingAtArmory(false);
      ln->PlanPathTo();
      return;
    }
    ln->setWaitingAtArmory(true);
  }
}

void GoToNeedyWarriorState::Transition(NPC *pn) {
  OnExit(pn);
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    pn->setCurrentState(new GiveAmmo());
    pn->getCurrentState()->OnEnter(pn);
    return;
  }
}

void GoToNeedyWarriorState::OnExit(NPC *pn) { pn->setIsMoving(false); }
