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
		// Don't interrupt fleeing warriors -- let them reach medic/supply
		if (warrior->isInRisk())
			return;

		NPC* enemy = warrior->FindEnemyInSameRoom();
		if (warrior->getAmmo() > 0 && enemy)
		{
			double ex, ey;
			enemy->getPosition(ex, ey);
			double targetCx = ex + 1.5;
			double targetCy = ey + 1.5;

			if (warrior->HasLineOfSight(targetCx, targetCy))
			{
				OnExit(pn);
				warrior->setCurrentState(new AttackState());
				warrior->getCurrentState()->OnEnter(pn);
			}
		}
	}
}

void MoveToTargetState::OnExit(NPC* pn)
{
	pn->setIsMoving(false);
}
