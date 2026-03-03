#include "SupplyNPC.h"
#include "FillAmmo.h"
#include "GiveAmmo.h"
#include "GoToWarrior.h"
#include "GoToArmory.h"
#include "Map.h"

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
      // Treat a warrior as needing ammo when they are noticeably below full.
      if (w->getHp() > 0 && w->getAmmo() < AMMO_MAX * 0.95) {
        if (w->getAmmo() < worstAmmo) {
          worstAmmo = w->getAmmo();
          worst = w;
        }
      }
    }
  }
  return worst;
}

WarriorNPC *SupplyNPC::FindWarriorWithLowestAmmo() {
  if (!myTeam)
    return nullptr;
  WarriorNPC *lowest = nullptr;
  double lowAmmo = AMMO_MAX;

  for (int i = 0; i < 2; i++) {
    if (auto w = dynamic_cast<WarriorNPC *>(myTeam[i])) {
      if (w->getHp() > 0 && w->getAmmo() < AMMO_MAX && w->getAmmo() < lowAmmo) {
        lowAmmo = w->getAmmo();
        lowest = w;
      }
    }
  }
  return lowest;
}

WarriorNPC *SupplyNPC::FindClosestWarrior() {
  if (!myTeam) return nullptr;
  WarriorNPC *closest = nullptr;
  double bestDist = 1e9;
  for (int i = 0; i < 2; i++) {
    if (auto w = dynamic_cast<WarriorNPC *>(myTeam[i])) {
      if (w->getHp() <= 0) continue;
      double wx, wy;
      w->getPosition(wx, wy);
      double dx = wx - x, dy = wy - y;
      double d = dx * dx + dy * dy;
      if (d < bestDist) { bestDist = d; closest = w; }
    }
  }
  return closest;
}

void SupplyNPC::DoSomeWork() {
  bool noPath = path.empty() || pathIndex < 0;
  bool possiblyStuck = !isGettingHp && !isFillingAmmo && !isGivingAmmo && noPath && isMoving;
  if (possiblyStuck) framesStuck++; else framesStuck = 0;

  if (framesStuck > 180 && goToWarrior && pWarrior) {
    framesStuck = 0;
    pWarrior = nullptr;
    goToWarrior = false;
    if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
    pCurrentState = new GoToArmory();
    pCurrentState->OnEnter(this);
  }

  // When being shot (low HP) or when seeing an enemy nearby, run toward closest warrior to avoid them
  bool shouldAvoid = isInRisk() || HasVisibleEnemyWithin(SUPPORT_AVOID_ENEMY_DIST);
  if (shouldAvoid && fleeFrames < 480) {
    fleeFrames++;
    WarriorNPC *closest = FindClosestWarrior();
    if (closest) {
      pWarrior = closest;
      setGoToWarrior(true);
      if (!dynamic_cast<GoToWarrior*>(pCurrentState)) {
        if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
        pCurrentState = new GoToWarrior();
        pCurrentState->OnEnter(this);
        if (!fleeMessageShown)
          fleeMessageShown = true;
      }
      double wx, wy;
      closest->getPosition(wx, wy);
      setTarget(wx, wy);
      isMoving = true;
      if (path.empty() || pathIndex < 0 || pathCooldown <= 0) {
        if (PlanPathTo() || PlanPathToIgnoreNPCs()) pathCooldown = 45;
        else pathCooldown = 20;
      }
      if (pathCooldown > 0) pathCooldown--;
      FollowPlannedPath(1);
    }
    return;
  }
  if (!shouldAvoid) fleeFrames = 0;
  fleeMessageShown = false;

  // If warrior target is dead or nearly full on ammo, find next target or go refill
  if (pWarrior && (pWarrior->getHp() <= 0 || pWarrior->getAmmo() >= AMMO_MAX * 0.95)) {
    pWarrior = nullptr;
    setGoToWarrior(false);
    supplyMessageShown = false;

    // If we still have ammo, find another warrior immediately instead of going to refill
    if (ammo > AMMO_MAX * 0.2) {
      WarriorNPC *next = FindWarriorNeedingAmmo();
      if (!next) next = FindWarriorWithLowestAmmo();
      if (next) {
        pWarrior = next;
        if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
        pCurrentState = new GoToWarrior();
        pCurrentState->OnEnter(this);
        return;
      }
    }
    if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
    pCurrentState = new GoToArmory();
    pCurrentState->OnEnter(this);
    return;
  }

  // Search for ammo at the beginning (GoToArmory state). After that, proactively search for soldiers to help.
  scanCooldown--;
  // After first fill, allow scanning even with relatively low own ammo and scan every frame.
  double ammoThreshold = stayedAtArmory ? AMMO_MAX * 0.1 : AMMO_MAX * 0.3;
  int cooldownAfterScan = stayedAtArmory ? 0 : 120;
  if (!pWarrior && ammo >= ammoThreshold && !isFillingAmmo && scanCooldown <= 0) {
    WarriorNPC *needy = FindWarriorNeedingAmmo();
    if (!needy)
      needy = FindWarriorWithLowestAmmo();  // move toward team even if someone is only slightly lower

    // New fallback: after refilling (stayedAtArmory), if no one qualifies by ammo
    // threshold, still move toward the closest living warrior instead of idling.
    if (!needy && stayedAtArmory) {
      needy = FindClosestWarrior();
    }

    if (needy) {
      pWarrior = needy;
      if (!supplyMessageShown)
        supplyMessageShown = true;

      if (pCurrentState) {
        pCurrentState->OnExit(this);
        delete pCurrentState;
      }
      pCurrentState = new GoToWarrior();
      pCurrentState->OnEnter(this);
    }
    scanCooldown = cooldownAfterScan;
  }

  if (pathCooldown > 0) pathCooldown--;
  if (isMoving && !isGettingHp) {
    double warrior_x, warrior_y;
    if (goToWarrior && pWarrior) {
      pWarrior->getPosition(warrior_x, warrior_y);
      setTarget(warrior_x, warrior_y);
      if (path.empty() || pathIndex < 0 || pathCooldown <= 0) {
        if (PlanPathTo() || PlanPathToIgnoreNPCs()) pathCooldown = 45;
        else pathCooldown = 20;
      }
    } else if (dynamic_cast<GoToArmory *>(pCurrentState) &&
               (path.empty() || pathIndex < 0) && pathCooldown <= 0 && numArmories > 0) {
      double px, py;
      getPosition(px, py);
      double bestDist = 99999.0;
      int bestX = armoryX[0], bestY = armoryY[0];
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
      setTarget(bestX, bestY);
      if (PlanPathTo() || PlanPathToIgnoreNPCs()) pathCooldown = 45;
      else pathCooldown = 5;  // refill at spawn: retry soon so we don't stand still
    }

    if (FollowPlannedPath(1)) {
      pCurrentState->Transition(this);
    }
  }

  if (isFillingAmmo) {
    // Claim depot on first fill frame
    if (fillingDepotIndex < 0) {
      double bestDist = 1e9;
      for (int i = 0; i < numArmories; i++) {
        double dx = x - armoryX[i];
        double dy = y - armoryY[i];
        double d = dx * dx + dy * dy;
        if (d < bestDist) { bestDist = d; fillingDepotIndex = i; }
      }
      if (fillingDepotIndex >= 0) {
        int occ = armoryOccupiedBy[fillingDepotIndex];
        if (occ != 0 && occ != team) {
          fillingDepotIndex = -1;
          isFillingAmmo = false;
          if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
          pCurrentState = new GoToArmory();
          pCurrentState->OnEnter(this);
        } else {
          armoryOccupiedBy[fillingDepotIndex] = team;
        }
      }
    }

    if (isFillingAmmo) {
      bool nearArmory = false;
      for (int i = 0; i < numArmories; i++) {
        double dx = x - armoryX[i];
        double dy = y - armoryY[i];
        if (dx * dx + dy * dy < 25.0) { nearArmory = true; break; }
      }
      if (!nearArmory) {
        if (fillingDepotIndex >= 0) { armoryOccupiedBy[fillingDepotIndex] = 0; fillingDepotIndex = -1; }
        isFillingAmmo = false;
        if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
        pCurrentState = new GoToArmory();
        pCurrentState->OnEnter(this);
      } else if (ammo < AMMO_MAX) {
        ammo += 0.1;
      } else {
        if (fillingDepotIndex >= 0) { armoryOccupiedBy[fillingDepotIndex] = 0; fillingDepotIndex = -1; }
        pCurrentState->Transition(this);
      }
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
    if (!isMoving && pWarrior && pWarrior->getAmmo() < AMMO_MAX) {
      pWarrior->setAmmo(pWarrior->getAmmo() + 0.1);
      ammo -= 0.1;
      if (ammo <= 0) {
        supplyMessageShown = false;
        pCurrentState->Transition(this);
      }
    } else {
      supplyMessageShown = false;
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
    glColor3d(1.0, 0.1, 0.1);
  else if (na < 0.60)
    glColor3d(1.0, 0.8, 0.0);
  else
    glColor3d(0.0, 1.0, 0.4);

  double fillH = barH * na;
  glBegin(GL_QUADS);
  glVertex2d(barX0, barY0);
  glVertex2d(barX0 + barW, barY0);
  glVertex2d(barX0 + barW, barY0 + fillH);
  glVertex2d(barX0, barY0 + fillH);
  glEnd();
}
