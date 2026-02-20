#include "GoToArmory.h"
#include "NPC.h"
#include "FillAmmo.h"
#include "Map.h"

void GoToArmory::OnEnter(NPC* pn)
{
	pn->setIsMoving(true);

	// Find the closest armory depot
	double px, py;
	pn->getPosition(px, py);
	double bestDist = 99999.0;
	int bestX = armoryX[0], bestY = armoryY[0];

	for (int i = 0; i < 2; i++) {
		double dx = armoryX[i] - px;
		double dy = armoryY[i] - py;
		double dist = dx * dx + dy * dy;
		if (dist < bestDist) {
			bestDist = dist;
			bestX = armoryX[i];
			bestY = armoryY[i];
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
