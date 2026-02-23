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
#include <string>

extern int map[MSZ][MSZ];

static const char* GetStateName(State* s) {
  if (!s) return "null";
  if (dynamic_cast<MoveToTargetState*>(s)) return "MoveToTarget";
  if (dynamic_cast<AttackState*>(s)) return "Attack";
  if (dynamic_cast<GoToDefenseState*>(s)) return "GoToDefense";
  if (dynamic_cast<IdleState*>(s)) return "Idle";
  return "Unknown";
}

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

  // Give an initial search target so warriors move out from spawn instead of standing still
  SearchForEnemies();
}

bool WarriorNPC::isInRisk() const {
  bool lowHp = hp < MAX_HP * hpFleeThreshold;
  bool lowAmmo = ammo < AMMO_MAX * ammoFleeThreshold;
  return lowHp || lowAmmo;
}

void WarriorNPC::setAmmo(double value) { ammo = value; }
double WarriorNPC::getAmmo() { return ammo; }
void WarriorNPC::setIsAttacking(bool value) { isAttacking = value; }

static const double NEARBY_ENEMY_RANGE = 20.0;

NPC *WarriorNPC::FindEnemyInSameRoom() {
  if (!enemyTeam)
    return nullptr;
  int myRoom = getCurrentRoom();

  NPC *closest = nullptr;
  double closestDist = 9999.0;

  for (int i = 0; i < TEAM_SIZE; i++) {
    if (!enemyTeam[i] || enemyTeam[i]->getHp() <= 0)
      continue;
    double ex, ey;
    enemyTeam[i]->getPosition(ex, ey);
    double d = Distance(x, y, ex, ey);
    if (d > NEARBY_ENEMY_RANGE)
      continue;
    bool inRange = false;
    if (myRoom > 0 && enemyTeam[i]->getCurrentRoom() == myRoom)
      inRange = true;
    else if (myRoom <= 0 || enemyTeam[i]->getCurrentRoom() <= 0)
      inRange = HasLineOfSight(ex + 1.5, ey + 1.5);
    else if (d < 8.0)
      inRange = HasLineOfSight(ex + 1.5, ey + 1.5);
    if (inRange && d < closestDist) {
      closestDist = d;
      closest = enemyTeam[i];
    }
  }
  return closest;
}

int WarriorNPC::CountEnemiesInSameRoom() {
  if (!enemyTeam) return 0;
  int myRoom = getCurrentRoom();
  int count = 0;
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (!enemyTeam[i] || enemyTeam[i]->getHp() <= 0)
      continue;
    double ex, ey;
    enemyTeam[i]->getPosition(ex, ey);
    double d = Distance(x, y, ex, ey);
    if (d > NEARBY_ENEMY_RANGE)
      continue;
    bool inRange = false;
    if (myRoom > 0 && enemyTeam[i]->getCurrentRoom() == myRoom)
      inRange = true;
    else if (myRoom <= 0 || enemyTeam[i]->getCurrentRoom() <= 0)
      inRange = HasLineOfSight(ex + 1.5, ey + 1.5);
    else if (d < 8.0)
      inRange = HasLineOfSight(ex + 1.5, ey + 1.5);
    if (inRange)
      count++;
  }
  return count;
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

  // Reset "message shown" when HP/ammo restored so we can announce again next time
  if (hpRatio >= hpFleeThreshold)
    lowHpMessageShown = false;
  if (ammoRatio >= ammoFleeThreshold)
    lowAmmoMessageShown = false;

  // Priority 1: critical HP ? seek medic first, else flee to cover
  if (hpRatio < hpFleeThreshold) {
    if (myTeam && myTeam[2] && myTeam[2]->getHp() > 0) {
      // Prefer going to medic (switch if we're not already going to medic; re-path to medic is done in DoSomeWork)
      if (!dynamic_cast<MoveToTargetState *>(pCurrentState)) {
        if (!lowHpMessageShown) {
          lowHpMessageShown = true;
          std::string color = (team == 1 ? TEAM1 : TEAM2);
          std::cout << color << "Warrior #" << npcType << " team " << team
                    << ": HP critical (" << (int)hp << "), going to MEDIC!" << RESET
                    << std::endl;
        }
        if (pCurrentState) {
          pCurrentState->OnExit(this);
          delete pCurrentState;
        }
        double mx, my;
        myTeam[2]->getPosition(mx, my);
        setTarget(mx, my);
        PlanPathTo();
        pCurrentState = new MoveToTargetState();
        pCurrentState->OnEnter(this);
      }
      return;
    }
    // No medic: flee to cover (stop shooting and run)
    if (!dynamic_cast<GoToDefenseState *>(pCurrentState)) {
      if (!lowHpMessageShown) {
        lowHpMessageShown = true;
        std::string color = (team == 1 ? TEAM1 : TEAM2);
        std::cout << color << "Warrior #" << npcType << " team " << team
                  << ": HP critical (" << (int)hp << "), FLEEING to cover!" << RESET
                  << std::endl;
      }
      setIsAttacking(false);  // ensure we stop shooting before fleeing
      if (pCurrentState) {
        pCurrentState->OnExit(this);
        delete pCurrentState;
      }
      pCurrentState = new GoToDefenseState();
      pCurrentState->OnEnter(this);
    }
    return;
  }

  // Priority 2: low ammo ? stop fighting and run to search for ammo guy (supply)
  if (ammoRatio < ammoFleeThreshold) {
    if (!dynamic_cast<MoveToTargetState *>(pCurrentState) ||
        !myTeam || !myTeam[3] || myTeam[3]->getHp() <= 0) {
      if (!lowAmmoMessageShown) {
        lowAmmoMessageShown = true;
        std::string color = (team == 1 ? TEAM1 : TEAM2);
        std::cout << color << "Warrior #" << npcType << " team " << team
                  << ": ammo low (" << (int)ammo << "), seeking supply!" << RESET
                  << std::endl;
      }

      if (pCurrentState) {
        pCurrentState->OnExit(this);
        delete pCurrentState;
      }

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
    }
    return;
  }

  // Priority 3: enemy in same room ? attack
  NPC *enemy = FindEnemyInSameRoom();
  if (enemy) {
    double ex, ey;
    enemy->getPosition(ex, ey);
    lastKnownEnemyX = ex + 1.5;
    lastKnownEnemyY = ey + 1.5;
  }
  if (enemy && ammo > 0 && !dynamic_cast<AttackState *>(pCurrentState)) {
    if (!attackingMessageShown) {
      attackingMessageShown = true;
      std::string color = (team == 1 ? TEAM1 : TEAM2);
      std::cout << color << "Warrior #" << npcType << " team " << team
                << ": ENEMY IN ROOM! Attacking!" << RESET << std::endl;
    }
    if (pCurrentState) {
      pCurrentState->OnExit(this);
      delete pCurrentState;
    }
    pCurrentState = new AttackState();
    pCurrentState->OnEnter(this);
    return;
  }

  // If attacking but enemy left room, chase toward last known position instead of random room
  if (dynamic_cast<AttackState *>(pCurrentState) && !enemy) {
    if (pCurrentState) {
      pCurrentState->OnExit(this);
      delete pCurrentState;
    }
    if (lastKnownEnemyX >= 0 && lastKnownEnemyY >= 0) {
      setTarget(lastKnownEnemyX, lastKnownEnemyY);
      PlanPathTo();
    }
    pCurrentState = new MoveToTargetState();
    pCurrentState->OnEnter(this);
    searchCooldown = 0;
    framesWithoutShooting = 0;
  }
}

static bool isFootprintWalkable(int tx, int ty) {
  for (int a = 0; a < 3; a++)
    for (int b = 0; b < 3; b++) {
      int i = tx + a, j = ty + b;
      if (i < 0 || i >= MSZ || j < 0 || j >= MSZ) return false;
      int c = map[i][j];
      if (c == WALL || c == STONE) return false;
    }
  return true;
}

void WarriorNPC::SearchForEnemies() {
  if (numRooms <= 0)
    return;

  int myRoom = getCurrentRoom();

  // Try multiple rooms to avoid getting stuck when pathfinding to one room fails
  for (int roomAttempt = 0; roomAttempt < 3; roomAttempt++) {
    int targetRoom;
    if (numRooms <= 1) {
      targetRoom = 1;
    } else {
      targetRoom = 1 + rand() % numRooms;
      if (targetRoom == myRoom) {
        targetRoom = 1 + (myRoom % numRooms);
      }
    }
    searchTargetRoom = targetRoom;

    Room *room = GetRoomById(targetRoom);
    if (!room) continue;

    int roomW = room->x2 - room->x1 + 1;
    int roomH = room->y2 - room->y1 + 1;
    int margin = 5;
    int rangeW = std::max(1, roomW - 2 * margin);
    int rangeH = std::max(1, roomH - 2 * margin);

    int tx = room->x1 + margin + (rangeW > 0 ? rand() % rangeW : 0);
    int ty = room->y1 + margin + (rangeH > 0 ? rand() % rangeH : 0);

    for (int attempt = 0; attempt < 8; attempt++) {
      if (tx + 2 < MSZ && ty + 2 < MSZ && isFootprintWalkable(tx, ty))
        break;
      tx = room->x1 + margin + (rangeW > 0 ? rand() % rangeW : 0);
      ty = room->y1 + margin + (rangeH > 0 ? rand() % rangeH : 0);
    }
    if (tx + 2 >= MSZ || ty + 2 >= MSZ || !isFootprintWalkable(tx, ty)) {
      tx = std::max(room->x1, std::min(room->x2 - 2, room->centerX() - 1));
      ty = std::max(room->y1, std::min(room->y2 - 2, room->centerY() - 1));
      if (!isFootprintWalkable(tx, ty)) continue;
    }

    setTarget(tx, ty);
    PlanPathTo();
    if (!path.empty()) return;

    PlanPathToIgnoreNPCs();
    if (!path.empty()) return;
  }
}

static int warrior_counter_grenade_frames = 0;

void WarriorNPC::DoSomeWork() {
  // Evaluate priorities (may force state transitions)
  EvaluatePriorities();

  if (pCurrentState)
    pCurrentState->Transition(this);

  // When critical and going to medic, re-path to medic periodically so we track them
  if (isInRisk() && myTeam && myTeam[2] && myTeam[2]->getHp() > 0 &&
      dynamic_cast<MoveToTargetState *>(pCurrentState)) {
    medicRepathFrames++;
    if (medicRepathFrames >= 30) {
      medicRepathFrames = 0;
      double mx, my;
      myTeam[2]->getPosition(mx, my);
      setTarget(mx, my);
      PlanPathTo();
    }
  } else {
    medicRepathFrames = 0;
  }

  if (isMoving && !isGettingHp) {
    arrivedAtTarget = false;
    searchCooldown--;
    if (searchCooldown < 0) searchCooldown = 0;
    // When path finished or empty, get new target (only when cooldown allows to avoid spam)
    if ((pathIndex < 0 || path.empty()) && searchCooldown <= 0) {
      if (!isInRisk() &&
          !dynamic_cast<AttackState *>(pCurrentState) &&
          !dynamic_cast<GoToDefenseState *>(pCurrentState)) {
        SearchForEnemies();
        searchCooldown = 15;
        // If still no path (e.g. stuck between wall and box), try without NPC avoidance to get unstuck
        if (path.empty() && targetX >= 0 && targetX < MSZ && targetY >= 0 && targetY < MSZ)
          PlanPathToIgnoreNPCs();
      }
    }
    if (FollowPlannedPath(0.15)) {
      arrivedAtTarget = true;
      framesAtTarget++;

      // If arrived and idle/moving, search for enemies in new room (not when critical HP)
      if (framesAtTarget > 60 && !isInRisk()) {
        framesAtTarget = 0;
        if (!dynamic_cast<AttackState *>(pCurrentState) &&
            !dynamic_cast<GoToDefenseState *>(pCurrentState) &&
            searchCooldown <= 0) {
          SearchForEnemies();
          searchCooldown = 45;
        }
      }
    }
  }

  if (!isMoving && !isGettingHp && !isAttacking) {
    framesAtTarget++;
    searchCooldown--;
    if (framesAtTarget > 120 && !isInRisk() && searchCooldown <= 0) {
      framesAtTarget = 0;
      SearchForEnemies();
      searchCooldown = 45;
      if (pCurrentState) {
        pCurrentState->OnExit(this);
        delete pCurrentState;
      }
      pCurrentState = new MoveToTargetState();
      pCurrentState->OnEnter(this);
    }
  }

  // Debug: detect stuck when in a moving state but no path or not moving
  bool inMovingState = dynamic_cast<MoveToTargetState*>(pCurrentState) || dynamic_cast<IdleState*>(pCurrentState) || dynamic_cast<GoToDefenseState*>(pCurrentState);
  bool noPath = path.empty() || pathIndex < 0;
  bool idleNoAttack = !isMoving && !isAttacking;
  bool possiblyStuck = !isGettingHp && inMovingState && (noPath || idleNoAttack);
  if (possiblyStuck) {
    framesStuck++;
    const char* reason = noPath ? (path.empty() ? "pathEmpty" : "pathIdx<0") : "idleNoAttack";
    if (framesStuck == 1) {
      std::string color = (team == 1 ? TEAM1 : TEAM2);
      std::cout << "[STUCK DEBUG] FIRST FRAME stuck reason=" << reason << " "
                << color << "Warrior #" << npcType << " team " << team << RESET
                << " state=" << GetStateName(pCurrentState)
                << " pos=(" << (int)x << "," << (int)y << ")"
                << " target=(" << (int)targetX << "," << (int)targetY << ")"
                << " pathSize=" << (int)path.size() << " pathIdx=" << pathIndex
                << " isMoving=" << isMoving << " isAttacking=" << isAttacking
                << " searchCooldown=" << searchCooldown << std::endl;
    } else if (framesStuck >= 45) {
      std::string color = (team == 1 ? TEAM1 : TEAM2);
      std::cout << "[STUCK DEBUG] still stuck (" << framesStuck << "f) " << color << "Warrior #" << npcType << " team " << team << RESET
                << " state=" << GetStateName(pCurrentState)
                << " pos=(" << (int)x << "," << (int)y << ")"
                << " pathSize=" << (int)path.size() << " pathIdx=" << pathIndex
                << " isMoving=" << isMoving << std::endl;
      framesStuck = 0;
    }
  } else {
    framesStuck = 0;
  }

  // Attacking logic: when 2+ enemies in room throw grenade (more damage); else shoot bullet
  // Never shoot when fleeing to cover (GoToDefenseState)
  if (isAttacking && !dynamic_cast<GoToDefenseState *>(pCurrentState)) {
    arrivedAtTarget = false;
    NPC *enemy = FindEnemyInSameRoom();
    int enemyCount = CountEnemiesInSameRoom();
    bool shotThisFrame = false;

    if (enemyCount >= 2 && pGrenade == nullptr && ammo >= 5 && !grenadeThrownThisRound) {
      double gx = 0, gy = 0;
      int n = 0;
      for (int i = 0; i < TEAM_SIZE; i++) {
        if (enemyTeam[i] && enemyTeam[i]->getHp() > 0 &&
            enemyTeam[i]->getCurrentRoom() == getCurrentRoom()) {
          double ex, ey;
          enemyTeam[i]->getPosition(ex, ey);
          gx += ex + 1.5;
          gy += ey + 1.5;
          n++;
        }
      }
      if (n > 0) {
        gx /= n;
        gy /= n;
        if (HasLineOfSight(gx, gy)) {
          pGrenade = new Grenade(gx, gy, team);
          grenadeThrownThisRound = true;
          std::cout << GRENADE << "Warrior #" << npcType << " team " << team
                    << ": " << enemyCount << " enemies in room - throwing GRENADE!" << RESET << std::endl;
          pGrenade->setIsExploded(true);
          ammo -= 5;
          shotThisFrame = true;
        }
      }
    } else if (enemy && ammo > 0 && pGrenade == nullptr) {
      double ex, ey;
      enemy->getPosition(ex, ey);
      double tx = ex + 1.5, ty = ey + 1.5;
      if (HasLineOfSight(tx, ty)) {
        double mx = x + 1.5, my = y + 1.5;
        double dx = tx - mx, dy = ty - my;
        double dist = sqrt(dx*dx + dy*dy);
        double ax = tx, ay = ty;
        if (dist > 0.5) {
          ax = tx - 0.25 * (dx / dist);
          ay = ty - 0.25 * (dy / dist);
          if (!HasLineOfSight(ax, ay)) { ax = tx; ay = ty; }
        }
        setAmmo(ammo - 0.01);
        dx = ax - mx; dy = ay - my;
        double angle = atan2(dy, dx);

        if (pBullet == nullptr) {
          pBullet = new Bullet(x + 1.5, y + 1.5, angle, team);
          pBullet->setIsMoving(true);
          shotThisFrame = true;
        }
      }
    }

    // Track frames without shooting to break combat stalls (LOS blocked by obstacle)
    if (shotThisFrame || pBullet != nullptr || pGrenade != nullptr) {
      framesWithoutShooting = 0;
    } else if (enemy) {
      framesWithoutShooting++;
    } else {
      framesWithoutShooting = 0;
    }

    // Reposition toward enemy when LOS is blocked for too long
    if (framesWithoutShooting > 45 && enemy) {
      double ex, ey;
      enemy->getPosition(ex, ey);
      double dx = (ex + 1.5) - (x + 1.5);
      double dy = (ey + 1.5) - (y + 1.5);
      double dist = sqrt(dx * dx + dy * dy);
      if (dist > 2.0) {
        double nx = x + (dx / dist) * SPEED;
        double ny = y + (dy / dist) * SPEED;
        int gi = (int)nx, gj = (int)ny;
        bool walkable = true;
        for (int a = 0; a < 3 && walkable; a++)
          for (int b = 0; b < 3 && walkable; b++) {
            int ci = gi + a, cj = gj + b;
            if (ci < 0 || ci >= MSZ || cj < 0 || cj >= MSZ ||
                map[ci][cj] == WALL || map[ci][cj] == STONE)
              walkable = false;
          }
        if (walkable) {
          x = nx;
          y = ny;
        }
      }
      framesWithoutShooting = 0;
    }
  }

  // Grenade when idle (single enemy in room, periodic) - max 1 per round
  if (auto idleState = dynamic_cast<IdleState *>(getCurrentState())) {
    if (warrior_counter_grenade_frames % 180 == 0) {
      NPC *enemy = FindEnemyInSameRoom();
      if (enemy && pGrenade == nullptr && ammo >= 5 && !grenadeThrownThisRound) {
        double ex, ey;
        enemy->getPosition(ex, ey);
        if (HasLineOfSight(ex + 1.5, ey + 1.5)) {
        pGrenade = new Grenade(ex + 1.5, ey + 1.5, team);
        grenadeThrownThisRound = true;
        std::cout << GRENADE << "Warrior #" << npcType << " team " << team
                  << " threw a grenade!" << RESET << std::endl;
        pGrenade->setIsExploded(true);
        ammo -= 5;
        }
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
