#pragma once
#include "State.h"
#include "NPC.h"
#include "LogisticNPC.h"

class GiveAmmo :
	public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};

