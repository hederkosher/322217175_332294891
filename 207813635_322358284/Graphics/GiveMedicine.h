#pragma once
#include "State.h"
#include "NPC.h"
#include "MedicNPC.h"

class GiveMedicine :
	public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};

