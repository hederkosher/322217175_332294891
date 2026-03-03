#include "GoToMedicine.h"
#include "FillMedicine.h"
#include "Map.h"
#include "NPC.h"


void GoToMedicine::OnEnter(NPC *pn) {
  pn->setIsMoving(true);

  int myTeam = pn->getTeam();
  double px, py;
  pn->getPosition(px, py);
  double bestDist = 99999.0;
  int bestX = medicineX[0], bestY = medicineY[0];
  bool found = false;

  for (int i = 0; i < numMedicine; i++) {
    // Auto-release depot if occupying team's medic NPC died
    if (medicineOccupiedBy[i] != 0 && medicineOccupiedBy[i] != myTeam) {
      NPC** enemy = pn->getEnemyTeam();
      bool enemyMedicAlive = enemy && enemy[2] && enemy[2]->getHp() > 0;
      if (!enemyMedicAlive)
        medicineOccupiedBy[i] = 0;
    }
    if (medicineOccupiedBy[i] != 0 && medicineOccupiedBy[i] != myTeam)
      continue;

    double dx = medicineX[i] - px;
    double dy = medicineY[i] - py;
    double dist = dx * dx + dy * dy;
    if (dist < bestDist) {
      bestDist = dist;
      bestX = medicineX[i];
      bestY = medicineY[i];
      found = true;
    }
  }

  if (!found) {
    for (int i = 0; i < numMedicine; i++) {
      double dx = medicineX[i] - px;
      double dy = medicineY[i] - py;
      double dist = dx * dx + dy * dy;
      if (dist < bestDist) {
        bestDist = dist;
        bestX = medicineX[i];
        bestY = medicineY[i];
      }
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
