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
		// Use room-based detection: stop attacking if no enemy in same room or out of ammo
		NPC* enemy = warrior->FindEnemyInSameRoom();
		if (warrior->getAmmo() <= 0 || !enemy)
		{
			OnExit(pn);
			// Chase toward last known enemy position instead of random room
			double lx = warrior->getLastKnownEnemyX();
			double ly = warrior->getLastKnownEnemyY();
			if (lx >= 0 && ly >= 0) {
				warrior->setTarget(lx, ly);
				warrior->PlanPathTo();
			}
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
		warrior->setAttackingMessageShown(false);  // allow "Attacking!" again next time
	}
}
