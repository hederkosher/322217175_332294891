#include "GiveMedicine.h"
#include "GoToMedicine.h"
#include "MedicNPC.h"
#include "NPC.h"


void GiveMedicine::OnEnter(NPC *pn) {
  if (auto mn = dynamic_cast<MedicNPC *>(pn)) {
    mn->setIsGivingMedicine(true);
    if (mn->getTargetNPC())
      mn->getTargetNPC()->setIsBeingHealed(true);
  }
}

void GiveMedicine::Transition(NPC *pn) {
  OnExit(pn);
  pn->setCurrentState(new GoToMedicine());
  pn->getCurrentState()->OnEnter(pn);
}

void GiveMedicine::OnExit(NPC *pn) {
  if (auto mn = dynamic_cast<MedicNPC *>(pn)) {
    mn->setIsGivingMedicine(false);
    mn->setGoToInjured(false);
    if (mn->getTargetNPC())
      mn->getTargetNPC()->setIsBeingHealed(false);
  }
}
