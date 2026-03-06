#include "IdleState.h"
#include "NPC.h"
#include "SupplyNPC.h"

void IdleState::OnEnter(NPC *pn) { pn->setIsMoving(false); }

void IdleState::Transition(NPC *pn) { OnExit(pn); }

void IdleState::OnExit(NPC *pn) {
  if (auto supply = dynamic_cast<SupplyNPC *>(pn)) {
    pn->setIsMoving(true);
  }
}
