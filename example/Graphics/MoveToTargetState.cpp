#include "MoveToTargetState.h"
#include <iostream> 

void MoveToTargetState::OnEnter(NPC* pn)
{
	pn->setIsMoving(true);
	if (auto warrior = dynamic_cast<WarriorNPC*>(pn))
	{
		warrior->setArrivedAtTarget(false);
		warrior->setIsAttacking(false);
	}
}

void MoveToTargetState::Transition(NPC* pn)
{
	if (auto warrior = dynamic_cast<WarriorNPC*>(pn))
	{
		double enemyX, enemyY;
		if (warrior->getAmmo() > 0 && warrior->FindVisibleEnemy(enemyX, enemyY))
		{
			OnExit(pn);
			warrior->setCurrentState(new AttackState());
			warrior->getCurrentState()->OnEnter(pn);
		}
	}
}

void MoveToTargetState::OnExit(NPC* pn)
{
	pn->setIsMoving(false);
}