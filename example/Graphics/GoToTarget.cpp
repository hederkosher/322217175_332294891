#include "GoToTarget.h"
#include "GiveMedicine.h"
#include "MedicNPC.h"
#include "NPC.h"


void GoToTarget::OnEnter(NPC *pn) {
  double x, y;
  if (auto mn = dynamic_cast<MedicNPC *>(pn)) {
    pn->setIsMoving(true);
    if (mn->getTargetNPC() && mn->getTargetNPC()->getHp() < MAX_HP / 2 &&
        mn->getTargetNPC()->getHp() > 0) {
      mn->setGoToTarget(true);
      mn->getTargetNPC()->getPosition(x, y);
      pn->setTarget(x, y);
      mn->setStayedAtMedicine(false);
      mn->PlanPathTo();
      return;
    }
    mn->setStayedAtMedicine(true);
  }
}

void GoToTarget::Transition(NPC *pn) {
  OnExit(pn);
  if (auto mn = dynamic_cast<MedicNPC *>(pn)) {
    pn->setCurrentState(new GiveMedicine());
    pn->getCurrentState()->OnEnter(pn);
    return;
  }
}

void GoToTarget::OnExit(NPC *pn) { pn->setIsMoving(false); }
