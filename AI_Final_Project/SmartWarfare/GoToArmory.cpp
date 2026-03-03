#include "GoToArmory.h"
#include "NPC.h"
#include "FillAmmo.h"
#include "Map.h"

void GoToArmory::OnEnter(NPC* pn)
{
	pn->setIsMoving(true);

	int myTeam = pn->getTeam();
	double px, py;
	pn->getPosition(px, py);
	double bestDist = 99999.0;
	int bestX = armoryX[0], bestY = armoryY[0];
	bool found = false;

	for (int i = 0; i < numArmories; i++) {
		// Auto-release depot if occupying team's supply NPC died
		if (armoryOccupiedBy[i] != 0 && armoryOccupiedBy[i] != myTeam) {
			NPC** enemy = pn->getEnemyTeam();
			bool enemySupplyAlive = enemy && enemy[3] && enemy[3]->getHp() > 0;
			if (!enemySupplyAlive)
				armoryOccupiedBy[i] = 0;
		}
		if (armoryOccupiedBy[i] != 0 && armoryOccupiedBy[i] != myTeam)
			continue;

		double dx = armoryX[i] - px;
		double dy = armoryY[i] - py;
		double dist = dx * dx + dy * dy;
		if (dist < bestDist) {
			bestDist = dist;
			bestX = armoryX[i];
			bestY = armoryY[i];
			found = true;
		}
	}

	if (!found) {
		// All depots occupied by enemy — go to the closest one anyway (wait for it)
		for (int i = 0; i < numArmories; i++) {
			double dx = armoryX[i] - px;
			double dy = armoryY[i] - py;
			double dist = dx * dx + dy * dy;
			if (dist < bestDist) {
				bestDist = dist;
				bestX = armoryX[i];
				bestY = armoryY[i];
			}
		}
	}

	pn->setTarget(bestX, bestY);
	pn->PlanPathTo();
}

void GoToArmory::Transition(NPC* pn)
{
	OnExit(pn);
	pn->setCurrentState(new FillAmmo());
	pn->getCurrentState()->OnEnter(pn);
}

void GoToArmory::OnExit(NPC* pn)
{
	pn->setIsMoving(false);
}
