#pragma once
#include "State.h"

class NPC;

class GoToTarget : public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};
