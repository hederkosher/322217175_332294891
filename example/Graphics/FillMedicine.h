#pragma once
#include "State.h"
#include "GoToTarget.h"

class NPC;

class FillMedicine : public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};
