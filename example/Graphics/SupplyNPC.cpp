#include "SupplyNPC.h"
#include "GoToWarrior.h"
#include <iostream>

static int counter = 0;

SupplyNPC::SupplyNPC(double positionX, double positionY, char character,
                     int team, int type)
    : NPC(positionX, positionY, character, team, type) {
  setCurrentState(new GoToArmory());
  getCurrentState()->OnEnter(this);
  setAmmo(AMMO_MAX / 10);
}

void SupplyNPC::setWarriorPointer(WarriorNPC *pW) { pWarrior = pW; }
WarriorNPC *SupplyNPC::getWarriorPointer() { return pWarrior; }
bool SupplyNPC::getGoToWarrior() { return goToWarrior; }
void SupplyNPC::setGoToWarrior(bool goToW) { goToWarrior = goToW; }
bool SupplyNPC::getIsGivingAmmo() { return isGivingAmmo; }
void SupplyNPC::setIsGivingAmmo(bool isGive) { isGivingAmmo = isGive; }
bool SupplyNPC::getIsFillingAmmo() { return isFillingAmmo; }
void SupplyNPC::setIsFillingAmmo(bool isFill) { isFillingAmmo = isFill; }
void SupplyNPC::setAmmo(double value) { ammo = value; }
bool SupplyNPC::getStayedAtArmory() { return stayedAtArmory; }
void SupplyNPC::setStayedAtArmory(bool stayed) { stayedAtArmory = stayed; }

WarriorNPC *SupplyNPC::FindWarriorNeedingAmmo() {
  if (!myTeam)
    return nullptr;
  WarriorNPC *worst = nullptr;
  double worstAmmo = AMMO_MAX;

  // Warriors are at indices 0 and 1
  for (int i = 0; i < 2; i++) {
    if (auto w = dynamic_cast<WarriorNPC *>(myTeam[i])) {
      if (w->getHp() > 0 && w->getAmmo() < AMMO_MAX / 3.0) {
        if (w->getAmmo() < worstAmmo) {
          worstAmmo = w->getAmmo();
          worst = w;
        }
      }
    }
  }
  return worst;
}

void SupplyNPC::DoSomeWork() {
  // Autonomous: if no warrior assigned and have ammo, scan warriors
  scanCooldown--;
  if (!pWarrior && ammo >= AMMO_MAX * 0.3 && !isFillingAmmo &&
      scanCooldown <= 0) {
    WarriorNPC *needy = FindWarriorNeedingAmmo();
    if (needy) {
      pWarrior = needy;
      std::string color = (team == 1 ? TEAM1 : TEAM2);
      std::cout << color << "Supply team " << team
                << ": detected warrior needing ammo, going to supply!" << RESET
                << std::endl;

      if (pCurrentState) {
        pCurrentState->OnExit(this);
        delete pCurrentState;
      }
      pCurrentState = new GoToWarrior();
      pCurrentState->OnEnter(this);
    }
    scanCooldown = 120;
  }

  if (isMoving && !isGettingHp) {
    double warrior_x, warrior_y;
    if (goToWarrior && pWarrior) {
      pWarrior->getPosition(warrior_x, warrior_y);
      setTarget(warrior_x, warrior_y);
      if (counter % 50 == 0)
        PlanPathTo();
      counter++;
    }

    if (FollowPlannedPath(1)) {
      pCurrentState->Transition(this);
    }
  }

  if (isFillingAmmo) {
    if (ammo < AMMO_MAX) {
      ammo += 0.1;
    } else if (pWarrior) {
      pCurrentState->Transition(this);
    }
  }

  if (stayedAtArmory) {
    if (pWarrior && pWarrior->getAmmo() < AMMO_MAX) {
      if (ammo >= AMMO_MAX)
        if (auto GoToWarriorState = dynamic_cast<GoToWarrior *>(pCurrentState))
          GoToWarriorState->OnEnter(this);
    }
  }

  if (isGivingAmmo) {
    if (pWarrior && pWarrior->getAmmo() < AMMO_MAX) {
      pWarrior->setAmmo(pWarrior->getAmmo() + 0.1);
      ammo -= 0.1;
      if (ammo <= 0)
        pCurrentState->Transition(this);
    } else {
      pCurrentState->Transition(this);
    }
  }
}

void SupplyNPC::show() {
  NPC::show();
  double size = 3.0;
  const double barW = 0.35;
  const double barH = size;
  const double barX0 = x - 0.6;
  const double barY0 = y;

  double na = ammo / AMMO_MAX;
  if (na < 0.0)
    na = 0.0;
  if (na > 1.0)
    na = 1.0;

  if (na < 0.20)
    glColor3d(1.0, 0.0, 0.0);
  else if (na < 0.60)
    glColor3d(0.0, 0.0, 0.6);
  else
    glColor3d(0.6, 0.0, 0.8);

  double fillH = barH * na;
  glBegin(GL_QUADS);
  glVertex2d(barX0, barY0);
  glVertex2d(barX0 + barW, barY0);
  glVertex2d(barX0 + barW, barY0 + fillH);
  glVertex2d(barX0, barY0 + fillH);
  glEnd();
}
