#include "SupplyNPC.h"
#include "GoToWarrior.h"
#include "GoToDefenseState.h"
#include "IdleState.h"
#include "MoveToTargetState.h"
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
double SupplyNPC::getAmmo() const { return ammo; }
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
      if (w->getHp() > 0 && w->getAmmo() < AMMO_MAX * 0.5) {
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
  // Priority 0: Self-preservation -- flee when HP is critically low
  double fleeThreshold = MAX_HP * 0.4;
  if (hp < fleeThreshold && hp > 0) {
    if (!isFleeing) {
      isFleeing = true;
      std::string color = (team == 1 ? TEAM1 : TEAM2);
      std::cout << color << "Supply team " << team
                << ": HP critical (" << (int)hp << "), retreating!" << RESET
                << std::endl;

      // Interrupt current activity
      isGivingAmmo = false;
      isFillingAmmo = false;
      stayedAtArmory = false;
      goToWarrior = false;
      pWarrior = nullptr;

      if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }

      // Flee to medic if alive, otherwise take cover
      if (myTeam && myTeam[2] && myTeam[2]->getHp() > 0) {
        double mx, my;
        myTeam[2]->getPosition(mx, my);
        setTarget(mx, my);
        PlanPathTo();
        fleeRepathCounter = 0;
        pCurrentState = new MoveToTargetState();
        pCurrentState->OnEnter(this);
      } else {
        pCurrentState = new GoToDefenseState();
        pCurrentState->OnEnter(this);
      }
    }

    // Periodically replan path to medic (medic moves)
    if (dynamic_cast<MoveToTargetState*>(pCurrentState) &&
        myTeam && myTeam[2] && myTeam[2]->getHp() > 0) {
      fleeRepathCounter++;
      if (fleeRepathCounter >= 50) {
        double mx, my;
        myTeam[2]->getPosition(mx, my);
        setTarget(mx, my);
        PlanPathTo();
        fleeRepathCounter = 0;
      }
    }

    // Follow path while fleeing — only "arrive" when in a room (never stop in corridor)
    if (isMoving && FollowPlannedPath(1)) {
      if (!IsInCorridor()) {
        if (pCurrentState)
          pCurrentState->Transition(this);
      } else {
        MoveToNearestRoom();
      }
    }
    return;
  }

  // Recovery: HP back above threshold
  if (isFleeing) {
    isFleeing = false;
    std::string color = (team == 1 ? TEAM1 : TEAM2);
    std::cout << color << "Supply team " << team
              << ": HP recovered, resuming duties." << RESET << std::endl;
    if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
    pCurrentState = new GoToArmory();
    pCurrentState->OnEnter(this);
  }

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
    scanCooldown = 30;
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

    // Only treat path as complete when in a room — never stop in corridor so enemies can engage
    if (FollowPlannedPath(1)) {
      if (!IsInCorridor()) {
        if (pCurrentState)
          pCurrentState->Transition(this);
      } else {
        MoveToNearestRoom();
      }
    }
  }

  if (isFillingAmmo) {
    if (ammo < AMMO_MAX) {
      ammo += 0.1;
    } else if (pWarrior) {
      pCurrentState->Transition(this);
    } else {
      // Ammo full but no warrior -- exit FillAmmo so scan code can run
      if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
      pCurrentState = nullptr;
      isFillingAmmo = false;
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

  // Follow nearest warrior when idle (not moving, not filling, not giving, has ammo)
  if (!isMoving && !isFillingAmmo && !isGivingAmmo && !goToWarrior
      && ammo >= AMMO_MAX * 0.3 && myTeam)
  {
    followCooldown--;
    int cooldownLimit = IsInCorridor() ? 0 : 60;
    if (followCooldown <= 0)
    {
      NPC* nearest = nullptr;
      double bestDist = 99999.0;
      for (int i = 0; i < 2; i++) {
        if (myTeam[i] && myTeam[i]->getHp() > 0) {
          double wx, wy;
          myTeam[i]->getPosition(wx, wy);
          double d = Distance(x, y, wx, wy);
          if (d < bestDist) {
            bestDist = d;
            nearest = myTeam[i];
          }
        }
      }
      if (nearest && bestDist > 8.0) {
        double wx, wy;
        nearest->getPosition(wx, wy);
        setTarget(wx, wy);
        PlanPathTo();
        isMoving = true;
      } else if (IsInCorridor()) {
        MoveToNearestRoom();
      }
      followCooldown = cooldownLimit;
    }
  }

  // Catch-all: never idle in a corridor
  if (!isMoving && IsInCorridor()) {
    MoveToNearestRoom();
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
    glColor3d(1.0, 0.1, 0.1);      // critical: red
  else if (na < 0.60)
    glColor3d(0.9, 0.7, 0.0);      // low: yellow
  else
    glColor3d(0.0, 0.8, 0.6);      // good: teal-green

  double fillH = barH * na;
  glBegin(GL_QUADS);
  glVertex2d(barX0, barY0);
  glVertex2d(barX0 + barW, barY0);
  glVertex2d(barX0 + barW, barY0 + fillH);
  glVertex2d(barX0, barY0 + fillH);
  glEnd();
}
