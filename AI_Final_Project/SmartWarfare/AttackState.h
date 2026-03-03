#pragma once
#include "State.h"
#include "NPC.h"
#include "WarriorNPC.h" 
#include "MoveToTargetState.h" 

class AttackState : public State
{
private:
	int framesWithoutEnemy = 0;
public:
	void OnEnter(NPC* pn) override;
	void Transition(NPC* pn) override;
	void OnExit(NPC* pn) override;
};
