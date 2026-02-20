#pragma once
#include "State.h"
#include "NPC.h"
#include "FillAmmo.h"


class GoToArmory :
	public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};

