#pragma once
#include "State.h"

class NPC;

class GoToNeedyWarriorState : public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};
