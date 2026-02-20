#pragma once
#include "State.h"

class NPC;

class IdleState : public State {
public:
  void OnEnter(NPC *pn) override;
  void Transition(NPC *pn) override;
  void OnExit(NPC *pn) override;
};
