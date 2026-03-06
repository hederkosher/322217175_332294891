#pragma once
#include "State.h"
#include "GoToInjuredState.h"

class NPC;

class FillMedicine : public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};
