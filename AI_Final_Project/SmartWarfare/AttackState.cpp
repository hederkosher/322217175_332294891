#include "AttackState.h"
#include "Definitions.h"
#include "GoToDefenseState.h"
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
		double hpRatio = warrior->getHp() / MAX_HP;

		// ammo == 0 -> MoveToTargetState (resupply aspiration)
		if (warrior->getAmmo() <= 0) {
			OnExit(pn);
			warrior->setCurrentState(new MoveToTargetState());
			warrior->getCurrentState()->OnEnter(pn);
			return;
		}
		// HP low -> run to cover; stop shooting (HP_NEED_HEAL_RATIO covers panic too)
		if (hpRatio < HP_NEED_HEAL_RATIO) {
			OnExit(pn);
			warrior->setCurrentState(new GoToDefenseState());
			warrior->getCurrentState()->OnEnter(pn);
			return;
		}

		if (!enemy) {
			framesWithoutEnemy++;
		} else {
			framesWithoutEnemy = 0;
		}

		// No enemy visible for N frames -> MoveToTargetState toward last-known (chase) or search
		if (framesWithoutEnemy >= CHASE_NO_ENEMY_FRAMES) {
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
		warrior->setIsAttacking(false);
}
