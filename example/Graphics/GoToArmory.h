#pragma once
#include "State.h"

class NPC;
class FillAmmo;

class GoToArmory : public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};
