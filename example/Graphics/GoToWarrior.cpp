#include "GoToWarrior.h"
#include "GiveAmmo.h"
#include "GoToArmory.h"
#include "NPC.h"
#include "SupplyNPC.h"


void GoToWarrior::OnEnter(NPC *pn) {
  double x, y;
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    if (ln->getWarriorPointer() && ln->getWarriorPointer()->getHp() > 0 &&
        ln->getWarriorPointer()->getAmmo() < AMMO_MAX * 0.6) {
      pn->setIsMoving(true);
      ln->setGoToWarrior(true);
      ln->getWarriorPointer()->getPosition(x, y);
      pn->setTarget(x, y);
      ln->setStayedAtArmory(false);
      ln->PlanPathTo();
      return;
    }
    ln->setWarriorPointer(nullptr);
    // After finding ammo (or no target): search for soldiers to help first
    WarriorNPC* needy = ln->FindWarriorNeedingAmmo();
    if (needy) {
      ln->setWarriorPointer(needy);
      pn->setIsMoving(true);
      ln->setGoToWarrior(true);
      needy->getPosition(x, y);
      pn->setTarget(x, y);
      ln->setStayedAtArmory(false);
      ln->PlanPathTo();
      return;
    }
    // No one below threshold: still move toward warrior with lowest ammo to be ready to resupply
    WarriorNPC* lowest = ln->FindWarriorWithLowestAmmo();
    if (lowest) {
      ln->setWarriorPointer(lowest);
      pn->setIsMoving(true);
      ln->setGoToWarrior(true);
      lowest->getPosition(x, y);
      pn->setTarget(x, y);
      ln->setStayedAtArmory(false);
      ln->PlanPathTo();
      return;
    }
    ln->setStayedAtArmory(true);
    pn->setIsMoving(false);
  }
}

void GoToWarrior::Transition(NPC *pn) {
  OnExit(pn);
  if (auto ln = dynamic_cast<SupplyNPC *>(pn)) {
    pn->setCurrentState(new GiveAmmo());
    pn->getCurrentState()->OnEnter(pn);
    return;
  }
}

void GoToWarrior::OnExit(NPC *pn) { pn->setIsMoving(false); }
