#include "GoToTarget.h"
#include "GiveMedicine.h"
#include "GoToMedicine.h"
#include "MedicNPC.h"
#include "NPC.h"


void GoToTarget::OnEnter(NPC *pn) {
  double x, y;
  if (auto mn = dynamic_cast<MedicNPC *>(pn)) {
    if (mn->getTargetNPC() && mn->getTargetNPC()->getHp() < MAX_HP &&
        mn->getTargetNPC()->getHp() > 0) {
      pn->setIsMoving(true);
      mn->setGoToTarget(true);
      mn->getTargetNPC()->getPosition(x, y);
      pn->setTarget(x, y);
      mn->setStayedAtMedicine(false);
      mn->PlanPathTo();
      return;
    }
    mn->setTargetNPC(nullptr);
    // After finding HP (or no target): search for soldiers to help first
    NPC* injured = mn->FindInjuredTeammate();
    if (injured) {
      mn->setTargetNPC(injured);
      pn->setIsMoving(true);
      mn->setGoToTarget(true);
      injured->getPosition(x, y);
      pn->setTarget(x, y);
      mn->setStayedAtMedicine(false);
      mn->PlanPathTo();
      return;
    }
    mn->setStayedAtMedicine(true);
    // No one to help: go refill at medicine depot
    pn->setCurrentState(new GoToMedicine());
    pn->getCurrentState()->OnEnter(pn);
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
