#include "GoToMedicine.h"
#include "FillMedicine.h"
#include "Map.h"
#include "NPC.h"


void GoToMedicine::OnEnter(NPC *pn) {
  pn->setIsMoving(true);

  // Find the closest medicine depot
  double px, py;
  pn->getPosition(px, py);
  double bestDist = 99999.0;
  int bestX = medicineX[0], bestY = medicineY[0];

  for (int i = 0; i < 2; i++) {
    double dx = medicineX[i] - px;
    double dy = medicineY[i] - py;
    double dist = dx * dx + dy * dy;
    if (dist < bestDist) {
      bestDist = dist;
      bestX = medicineX[i];
      bestY = medicineY[i];
    }
  }

  pn->setTarget(bestX, bestY);
  pn->PlanPathTo();
}

void GoToMedicine::Transition(NPC *pn) {
  OnExit(pn);
  pn->setCurrentState(new FillMedicine());
  pn->getCurrentState()->OnEnter(pn);
}

void GoToMedicine::OnExit(NPC *pn) { pn->setIsMoving(false); }
