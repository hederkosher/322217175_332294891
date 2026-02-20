#include "AttackState.h"
#include <iostream> 

void AttackState::OnEnter(NPC* pn)
{
	if (auto warrior = dynamic_cast<WarriorNPC*>(pn))
	{
		warrior->setArrivedAtTarget(false);
		warrior->setIsAttacking(true);
		warrior->setIsMoving(false);
	}
}

void AttackState::Transition(NPC* pn)
{
	if (auto warrior = dynamic_cast<WarriorNPC*>(pn))
	{
		double enemyX, enemyY;
		if (warrior->getAmmo() <= 0 || !warrior->FindVisibleEnemy(enemyX, enemyY))
		{
			OnExit(pn);
			warrior->setCurrentState(new MoveToTargetState());
			warrior->getCurrentState()->OnEnter(pn);
		}
	}
}

void AttackState::OnExit(NPC* pn)
{
	if (auto warrior = dynamic_cast<WarriorNPC*>(pn))
	{
		warrior->setIsAttacking(false);
	}
}