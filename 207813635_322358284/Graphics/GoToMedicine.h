#pragma once
#include "State.h"
#include "NPC.h"
#include "FillMedicine.h"


class GoToMedicine :
	public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};

