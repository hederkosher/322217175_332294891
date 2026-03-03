#pragma once
#include "State.h"
#include "GoToWarrior.h"

class NPC;

class FillAmmo : public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};
