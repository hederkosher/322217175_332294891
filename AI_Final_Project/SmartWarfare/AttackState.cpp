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
		NPC* enemy = warrior->FindEnemyInSameRoom();

		if (warrior->getAmmo() <= 0) {
			OnExit(pn);
			warrior->setCurrentState(new MoveToTargetState());
			warrior->getCurrentState()->OnEnter(pn);
			return;
		}

		if (!enemy) {
			framesWithoutEnemy++;
		} else {
			framesWithoutEnemy = 0;
		}

		// Only exit attack after enemy has been gone for a grace period
		if (framesWithoutEnemy >= 30) {
			OnExit(pn);
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
	}
}
