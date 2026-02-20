#pragma once
#include "State.h"
#include "NPC.h"
#include "LogisticNPC.h"
#include "GiveAmmo.h"
#include "MedicNPC.h"
#include "GiveMedicine.h"


class GoToWarrior :
	public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);
};

