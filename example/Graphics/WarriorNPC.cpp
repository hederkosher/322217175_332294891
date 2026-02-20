#include "WarriorNPC.h"
#include "AttackState.h"
#include "Bullet.h"
#include "GoToDefenseState.h"
#include "IdleState.h"
#include "Map.h"
#include "MoveToTargetState.h"
#include <algorithm>
#include <iostream>
#include <math.h>


extern int map[MSZ][MSZ];

WarriorNPC::WarriorNPC(double positionX, double positionY, char character,
                       int team, int type)
    : NPC(positionX, positionY, character, team, type) {
  hp = MAX_HP;
  ammo = AMMO_MAX;
  pathIndex = -1;
  isAttacking = false;

  // Personality-derived thresholds
  hpFleeThreshold = 0.25 + 0.2 * cautiousness - 0.15 * aggressiveness;
  ammoFleeThreshold = 0.15 + 0.15 * cautiousness - 0.1 * aggressiveness;
  if (hpFleeThreshold < 0.1)
    hpFleeThreshold = 0.1;
  if (ammoFleeThreshold < 0.05)
    ammoFleeThreshold = 0.05;

  std::string color = (team == 1 ? TEAM1 : TEAM2);
  std::cout << color << "  Warrior thresholds: hpFlee=" << hpFleeThreshold
            << " ammoFlee=" << ammoFleeThreshold << RESET << std::endl;

  setCurrentState(new MoveToTargetState());
  getCurrentState()->OnEnter(this);
}

bool WarriorNPC::isInRisk() const {
  bool lowHp = hp < MAX_HP * hpFleeThreshold;
  bool lowAmmo = ammo < AMMO_MAX * ammoFleeThreshold;
  return lowHp || lowAmmo;
}

void WarriorNPC::setAmmo(double value) { ammo = value; }
double WarriorNPC::getAmmo() { return ammo; }
void WarriorNPC::setIsAttacking(bool value) { isAttacking = value; }

NPC *WarriorNPC::FindEnemyInSameRoom() {
  if (!enemyTeam)
    return nullptr;
  int myRoom = getCurrentRoom();
  if (myRoom <= 0)
    return nullptr;

  NPC *closest = nullptr;
  double closestDist = 9999.0;

  for (int i = 0; i < TEAM_SIZE; i++) {
    if (enemyTeam[i] && enemyTeam[i]->getHp() > 0) {
      if (enemyTeam[i]->getCurrentRoom() == myRoom) {
        double ex, ey;
        enemyTeam[i]->getPosition(ex, ey);
        double d = Distance(x, y, ex, ey);
        if (d < closestDist) {
          closestDist = d;
          closest = enemyTeam[i];
        }
      }
    }
  }
  return closest;
}

bool WarriorNPC::FindVisibleEnemy(double &outX, double &outY) {
  for (int i = 0; i < MSZ; i++) {
    for (int j = 0; j < MSZ; j++) {
      if (visibilityMap[i][j] > 0) {
        outX = i;
        outY = j;
        return true;
      }
    }
  }
  return false;
}

void WarriorNPC::EvaluatePriorities() {
  if (isGettingHp)
    return;

  double hpRatio = hp / MAX_HP;
  double ammoRatio = ammo / AMMO_MAX;

  // Priority 1: critical HP ? flee to cover or seek medic
  if (hpRatio < hpFleeThreshold) {
    if (!dynamic_cast<GoToDefenseState *>(pCurrentState)) {
      std::string color = (team == 1 ? TEAM1 : TEAM2);
      std::cout << color << "Warrior #" << npcType << " team " << team
                << ": HP critical (" << (int)hp << "), FLEEING!" << RESET
                << std::endl;

      if (pCurrentState) {
        pCurrentState->OnExit(this);
        delete pCurrentState;
      }

      // Try to go to medic if available
      if (myTeam && myTeam[2] && myTeam[2]->getHp() > 0) {
        double mx, my;
        myTeam[2]->getPosition(mx, my);
        setTarget(mx, my);
        PlanPathTo();
        pCurrentState = new MoveToTargetState();
        pCurrentState->OnEnter(this);
      } else {
        pCurrentState = new GoToDefenseState();
        pCurrentState->OnEnter(this);
      }
      return;
    }
  }

  // Priority 2: low ammo ? seek supply
  if (ammoRatio < ammoFleeThreshold && isAttacking) {
    std::string color = (team == 1 ? TEAM1 : TEAM2);
    std::cout << color << "Warrior #" << npcType << " team " << team
              << ": ammo low (" << (int)ammo << "), seeking supply!" << RESET
              << std::endl;

    if (pCurrentState) {
      pCurrentState->OnExit(this);
      delete pCurrentState;
    }

    // Go to supply NPC if available
    if (myTeam && myTeam[3] && myTeam[3]->getHp() > 0) {
      double lx, ly;
      myTeam[3]->getPosition(lx, ly);
      setTarget(lx, ly);
      PlanPathTo();
      pCurrentState = new MoveToTargetState();
      pCurrentState->OnEnter(this);
    } else {
      pCurrentState = new IdleState();
      pCurrentState->OnEnter(this);
    }
    return;
  }

  // Priority 3: enemy in same room ? attack
  NPC *enemy = FindEnemyInSameRoom();
  if (enemy && ammo > 0 && !dynamic_cast<AttackState *>(pCurrentState)) {
    std::string color = (team == 1 ? TEAM1 : TEAM2);
    std::cout << color << "Warrior #" << npcType << " team " << team
              << ": ENEMY IN ROOM! Attacking!" << RESET << std::endl;

    if (pCurrentState) {
      pCurrentState->OnExit(this);
      delete pCurrentState;
    }
    pCurrentState = new AttackState();
    pCurrentState->OnEnter(this);
    return;
  }

  // If attacking but enemy left room, stop attacking
  if (dynamic_cast<AttackState *>(pCurrentState) && !enemy) {
    if (pCurrentState) {
      pCurrentState->OnExit(this);
      delete pCurrentState;
    }
    pCurrentState = new MoveToTargetState();
    pCurrentState->OnEnter(this);
  }
}

void WarriorNPC::SearchForEnemies() {
  if (numRooms <= 0)
    return;

  // Pick a random room to explore
  int targetRoom = 1 + rand() % numRooms;
  int attempts = 0;
  while (targetRoom == getCurrentRoom() && attempts < 10) {
    targetRoom = 1 + rand() % numRooms;
    attempts++;
  }
  searchTargetRoom = targetRoom;

  Room *room = GetRoomById(targetRoom);
  if (room) {
    int tx = room->x1 + 3 + rand() % max(1, room->x2 - room->x1 - 6);
    int ty = room->y1 + 3 + rand() % max(1, room->y2 - room->y1 - 6);
    setTarget(tx, ty);
    PlanPathTo();

    std::string color = (team == 1 ? TEAM1 : TEAM2);
    std::cout << color << "Warrior #" << npcType << " team " << team
              << ": searching room " << targetRoom << " at (" << tx << "," << ty
              << ")" << RESET << std::endl;
  }
}

static int warrior_counter_grenade_frames = 0;

void WarriorNPC::DoSomeWork() {
  // Evaluate priorities (may force state transitions)
  EvaluatePriorities();

  if (pCurrentState)
    pCurrentState->Transition(this);

  if (isMoving && !isGettingHp) {
    arrivedAtTarget = false;
    if (FollowPlannedPath(0.15)) {
      arrivedAtTarget = true;
      framesAtTarget++;

      // If arrived and idle/moving, search for enemies in new room
      if (framesAtTarget > 60) {
        framesAtTarget = 0;
        if (!dynamic_cast<AttackState *>(pCurrentState) &&
            !dynamic_cast<GoToDefenseState *>(pCurrentState)) {
          SearchForEnemies();
        }
      }
    }
  }

  if (!isMoving && !isGettingHp && !isAttacking) {
    framesAtTarget++;
    if (framesAtTarget > 120) {
      framesAtTarget = 0;
      SearchForEnemies();
      if (pCurrentState) {
        pCurrentState->OnExit(this);
        delete pCurrentState;
      }
      pCurrentState = new MoveToTargetState();
      pCurrentState->OnEnter(this);
    }
  }

  // Attacking logic: only fire when enemy is in the same room
  if (isAttacking) {
    arrivedAtTarget = false;
    NPC *enemy = FindEnemyInSameRoom();
    if (enemy && ammo > 0) {
      setAmmo(ammo - 0.01);

      double ex, ey;
      enemy->getPosition(ex, ey);
      double dx = (ex + 1.5) - (x + 1.5);
      double dy = (ey + 1.5) - (y + 1.5);
      double angle = atan2(dy, dx);

      if (pBullet == nullptr) {
        pBullet = new Bullet(x + 1.5, y + 1.5, angle, team);
        pBullet->setIsMoving(true);
      }
    }
  }

  // Grenade throwing (periodic, when idle and have ammo)
  if (auto idleState = dynamic_cast<IdleState *>(getCurrentState())) {
    if (warrior_counter_grenade_frames % 180 == 0) {
      NPC *enemy = FindEnemyInSameRoom();
      if (enemy && pGrenade == nullptr && ammo >= 5) {
        double ex, ey;
        enemy->getPosition(ex, ey);
        pGrenade = new Grenade(ex + 1.5, ey + 1.5, team);
        std::cout << GRENADE << "Warrior #" << npcType << " team " << team
                  << " threw a grenade!" << RESET << std::endl;
        pGrenade->setIsExploded(true);
        ammo -= 5;
      }
    }
    warrior_counter_grenade_frames++;
  }
}

void WarriorNPC::show() {
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

Bullet *WarriorNPC::getBullet() const { return pBullet; }
void WarriorNPC::setBullet(Bullet *bullet) { pBullet = bullet; }
Grenade *WarriorNPC::getGrenade() const { return pGrenade; }
void WarriorNPC::setGrenade(Grenade *grenade) { pGrenade = grenade; }
