#include "WarriorNPC.h"
#include "AttackState.h"
#include "Bullet.h"
#include "GoToDefenseState.h"
#include "IdleState.h"
#include "Map.h"
#include "MoveToTargetState.h"
#include "glut.h"
#include <algorithm>
#include <math.h>
#include <string>

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

// Explicit aspiration selection (spec: one goal at a time, high to low priority)
WarriorNPC::Aspiration WarriorNPC::SelectAspiration() {
  double hpRatio = hp / MAX_HP;
  double ammoRatio = ammo / AMMO_MAX;
  // (1) HP < HP_panic -> Escape to cover
  if (hpRatio < hpFleeThreshold)
    return ASPIRATION_ESCAPE;
  // (2) HP < HP_needHeal -> target Medic if reachable, else escape to cover (don't keep fighting when hurt)
  if (hpRatio < HP_NEED_HEAL_RATIO) {
    if (myTeam && myTeam[2] && myTeam[2]->getHp() > 0 && medicGiveUpFrames <= 0)
      return ASPIRATION_HEAL;
    return ASPIRATION_ESCAPE;  // medic dead or gave up -> run to cover, don't keep shooting
  }
  // (3) ammo < Ammo_needResupply -> target Supply
  if (ammoRatio < ammoFleeThreshold && myTeam && myTeam[3] && myTeam[3]->getHp() > 0)
    return ASPIRATION_RESUPPLY;
  // (4) enemy known -> fight (pursue last-known / same room); else search
  NPC* enemy = FindEnemyInSameRoom();
  if (enemy) {
    double ex, ey;
    enemy->getPosition(ex, ey);
    lastKnownEnemyX = ex + 1.5;
    lastKnownEnemyY = ey + 1.5;
    return ASPIRATION_FIGHT;
  }
  if (lastKnownEnemyX >= 0 && lastKnownEnemyY >= 0)
    return ASPIRATION_FIGHT;  // chase last-known
  return ASPIRATION_SEARCH;
}

void WarriorNPC::ApplyAspiration(Aspiration a) {
  if (a == ASPIRATION_ESCAPE) {
    // Stuck in GoToDefenseState (BFS failed or no path) -> resume fight/search
    if (dynamic_cast<GoToDefenseState*>(pCurrentState) && (path.empty() || pathIndex < 0) && !getIsMoving() && framesStuck > 90) {
      if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
      pCurrentState = new MoveToTargetState();
      pCurrentState->OnEnter(this);
      SearchForEnemies();
      framesStuck = 0;
      return;
    }
    // Already at cover (IdleState) or fleeing to cover (GoToDefenseState) -> stay
    if (dynamic_cast<GoToDefenseState*>(pCurrentState) || dynamic_cast<IdleState*>(pCurrentState))
      return;
    if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
    pCurrentState = new GoToDefenseState();
    pCurrentState->OnEnter(this);
    return;
  }
  if (a == ASPIRATION_HEAL) {
    if (!dynamic_cast<MoveToTargetState*>(pCurrentState) || path.empty()) {
      double mx, my;
      myTeam[2]->getPosition(mx, my);
      setTarget(mx, my);
      PlanPathTo();
      if (path.empty()) PlanPathToIgnoreNPCs();
      if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
      pCurrentState = new MoveToTargetState();
      pCurrentState->OnEnter(this);
    }
    return;
  }
  if (a == ASPIRATION_RESUPPLY) {
    if (!dynamic_cast<MoveToTargetState*>(pCurrentState) || path.empty()) {
      if (myTeam[3]) {
        double lx, ly;
        myTeam[3]->getPosition(lx, ly);
        setTarget(lx, ly);
        PlanPathTo();
      }
      if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
      pCurrentState = new MoveToTargetState();
      pCurrentState->OnEnter(this);
      if (path.empty()) SearchForEnemies();
    }
    return;
  }
  if (a == ASPIRATION_FIGHT) {
    // MoveToTargetState::Transition will switch to AttackState when enemy in same room + LOS + ammo
    if (lastKnownEnemyX >= 0 && lastKnownEnemyY >= 0 && !dynamic_cast<AttackState*>(pCurrentState)) {
      bool inMoving = dynamic_cast<MoveToTargetState*>(pCurrentState);
      if (!inMoving && !dynamic_cast<GoToDefenseState*>(pCurrentState)) {
        setTarget(lastKnownEnemyX, lastKnownEnemyY);
        PlanPathTo();
        if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
        pCurrentState = new MoveToTargetState();
        pCurrentState->OnEnter(this);
      } else if (inMoving && (path.empty() || pathIndex < 0)) {
        setTarget(lastKnownEnemyX, lastKnownEnemyY);
        PlanPathTo();
      }
    }
    return;
  }
  // ASPIRATION_SEARCH
  if (!dynamic_cast<MoveToTargetState*>(pCurrentState) || (path.empty() && searchCooldown <= 0)) {
    SearchForEnemies();
    if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
    pCurrentState = new MoveToTargetState();
    pCurrentState->OnEnter(this);
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
    if (path.size() > 2) return;

    // PlanPathTo may have only created a short escape path - clear and try clean A*
    path.clear();
    pathIndex = -1;
    PlanPathToIgnoreNPCs();
    if (!path.empty()) return;
  }
}

static int warrior_counter_grenade_frames = 0;

void WarriorNPC::DoSomeWork() {
  // Grenade evasion: detect nearby enemy grenades with active fuse and flee
  const double GRENADE_DANGER_RADIUS = 22.0;
  double nearestGrenDist = 1e9;
  double grenX = 0, grenY = 0;
  bool grenadeNearby = false;

  if (enemyTeam) {
    for (int i = 0; i < 2; i++) {
      if (auto ew = dynamic_cast<WarriorNPC*>(enemyTeam[i])) {
        Grenade* g = ew->getGrenade();
        if (g && !g->getIsFlying() && g->getFuseTimer() > 0) {
          double gx = g->getTargetX(), gy = g->getTargetY();
          double dx = gx - (x + 1.5), dy = gy - (y + 1.5);
          double dist = sqrt(dx * dx + dy * dy);
          if (dist < GRENADE_DANGER_RADIUS && dist < nearestGrenDist) {
            nearestGrenDist = dist;
            grenX = gx;
            grenY = gy;
            grenadeNearby = true;
          }
        }
      }
    }
  }

  if (grenadeNearby) {
    fleeingGrenade = true;
    isAttacking = false;
    double mx = x + 1.5, my = y + 1.5;
    double dx = mx - grenX, dy = my - grenY;
    double dist = sqrt(dx * dx + dy * dy);
    if (dist > 0.1) {
      double nx = x + (dx / dist) * SPEED * 2.0;
      double ny = y + (dy / dist) * SPEED * 2.0;
      int gi = (int)nx, gj = (int)ny;
      bool walkable = true;
      for (int a = 0; a < 3 && walkable; a++)
        for (int b = 0; b < 3 && walkable; b++) {
          int ci = gi + a, cj = gj + b;
          if (ci < 0 || ci >= MSZ || cj < 0 || cj >= MSZ ||
              map[ci][cj] == WALL || map[ci][cj] == STONE)
            walkable = false;
        }
      if (walkable) { x = nx; y = ny; }
    }
    return;
  } else if (fleeingGrenade) {
    fleeingGrenade = false;
  }

  // Explicit aspiration selection (spec: one goal per tick)
  ApplyAspiration(SelectAspiration());

  if (pCurrentState)
    pCurrentState->Transition(this);

  // Tick down medicGiveUpFrames cooldown
  if (medicGiveUpFrames > 0) medicGiveUpFrames--;

  // When critical and going to medic, re-path to medic periodically so we track them
  if (isInRisk() && myTeam && myTeam[2] && myTeam[2]->getHp() > 0 &&
      medicGiveUpFrames <= 0 && dynamic_cast<MoveToTargetState *>(pCurrentState)) {
    medicRepathFrames++;
    // Only repath when we have no path or are stuck; avoid replacing path every 30 frames (caused giggle)
    if (medicRepathFrames >= 30 && (path.empty() || framesStuck > 30)) {
      medicRepathFrames = 0;
      double mx, my;
      myTeam[2]->getPosition(mx, my);
      setTarget(mx, my);
      PlanPathTo();
      if (path.empty() && targetX >= 0 && targetX < MSZ && targetY >= 0 && targetY < MSZ)
        PlanPathToIgnoreNPCs();
    }
    // If stuck trying to reach medic for too long, give up and fight for a while
    if (framesStuck > 120 && path.empty()) {
      medicGiveUpFrames = 300;
      framesStuck = 0;
      medicRepathFrames = 0;
      if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
      pCurrentState = new MoveToTargetState();
      pCurrentState->OnEnter(this);
      SearchForEnemies();
    }
  } else {
    medicRepathFrames = 0;
  }

  if (isMoving) {
    arrivedAtTarget = false;
    searchCooldown--;
    if (searchCooldown < 0) searchCooldown = 0;
    bool pathEmpty = pathIndex < 0 || path.empty();
    if (pathEmpty && searchCooldown <= 0) {
      if (!isInRisk() &&
          !dynamic_cast<AttackState *>(pCurrentState) &&
          !dynamic_cast<GoToDefenseState *>(pCurrentState)) {
        SearchForEnemies();
        searchCooldown = 15;
        if (path.empty() && targetX >= 0 && targetX < MSZ && targetY >= 0 && targetY < MSZ)
          PlanPathToIgnoreNPCs();
      } else if (isInRisk() && dynamic_cast<MoveToTargetState *>(pCurrentState) &&
                 myTeam && myTeam[2] && myTeam[2]->getHp() > 0 && path.empty()) {
        // Going to medic but path failed - retry with fallback (no NPC avoidance); only when we have no path
        double mx, my;
        myTeam[2]->getPosition(mx, my);
        setTarget(mx, my);
        PlanPathTo();
        if (path.empty() && targetX >= 0 && targetX < MSZ && targetY >= 0 && targetY < MSZ)
          PlanPathToIgnoreNPCs();
        searchCooldown = 10;
      }
    }
    if (FollowPlannedPath(0.45)) {  // larger threshold = fewer waypoint flips, less giggle
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

  if (!isMoving && !isAttacking) {
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

  bool inMovingState = dynamic_cast<MoveToTargetState*>(pCurrentState) || dynamic_cast<IdleState*>(pCurrentState) || dynamic_cast<GoToDefenseState*>(pCurrentState);
  bool noPath = path.empty() || pathIndex < 0;
  bool idleNoAttack = !isMoving && !isAttacking;
  bool possiblyStuck = inMovingState && (noPath || idleNoAttack);
  if (possiblyStuck)
    framesStuck++;
  else
    framesStuck = 0;

  // Fire mode (spec): grenade if (enemy behind cover OR >=2 enemies in blast OR enemy very close) and self at safe distance; else bullet
  // Never shoot when fleeing to cover (GoToDefenseState)
  if (isAttacking && !dynamic_cast<GoToDefenseState *>(pCurrentState)) {
    arrivedAtTarget = false;
    NPC *enemy = FindEnemyInSameRoom();
    int enemyCount = CountEnemiesInSameRoom();
    bool shotThisFrame = false;
    double ex = 0, ey = 0;
    if (enemy) enemy->getPosition(ex, ey);
    double tx = ex + 1.5, ty = ey + 1.5;
    double mx = x + 1.5, my = y + 1.5;
    double distToEnemy = enemy ? sqrt((tx - mx)*(tx - mx) + (ty - my)*(ty - my)) : 999.0;
    bool enemyBehindCover = enemy && !HasLineOfSight(tx, ty);
    bool twoOrMoreInBlast = enemyCount >= 2;
    bool enemyVeryClose = distToEnemy < 6.0;
    const double GRENADE_SAFE_DIST = 5.0;
    bool useGrenade = (enemyBehindCover || twoOrMoreInBlast || enemyVeryClose) && ammo >= 5 && !pGrenade && !grenadeThrownThisRound && distToEnemy >= GRENADE_SAFE_DIST;

    if (useGrenade) {
      double gx = tx, gy = ty;
      if (twoOrMoreInBlast) {
        gx = gy = 0;
        int n = 0;
        for (int i = 0; i < TEAM_SIZE; i++) {
          if (enemyTeam[i] && enemyTeam[i]->getHp() > 0 && enemyTeam[i]->getCurrentRoom() == getCurrentRoom()) {
            double eex, eey;
            enemyTeam[i]->getPosition(eex, eey);
            gx += eex + 1.5; gy += eey + 1.5; n++;
          }
        }
        if (n > 0) { gx /= n; gy /= n; }
      }
      double dx = gx - mx, dy = gy - my;
      double d = sqrt(dx*dx + dy*dy);
      double tgx = gx, tgy = gy;
      if (d > 0.5) {
        double perpX = -dy / d, perpY = dx / d;
        double offset = 2.5 + (rand() % 100) / 50.0;
        int side = (rand() % 2) ? 1 : -1;
        tgx = gx + side * perpX * offset; tgy = gy + side * perpY * offset;
      }
      if (HasLineOfSight(tgx, tgy)) {
        pGrenade = new Grenade(mx, my, tgx, tgy, team);
        grenadeThrownThisRound = true;
        if (grenadeMsgCooldown <= 0) grenadeMsgCooldown = 300;
        ammo -= 5;
        shotThisFrame = true;
      }
    }
    if (!shotThisFrame && enemy && ammo > 0 && pGrenade == nullptr) {
      if (HasLineOfSight(tx, ty)) {
        double dx = tx - mx, dy = ty - my;
        double dist = sqrt(dx*dx + dy*dy);
        if (dist < 0.001) dist = 0.001;
        double ax = tx, ay = ty;
        if (dist > 0.5) {
          ax = tx - 0.25 * (dx / dist);
          ay = ty - 0.25 * (dy / dist);
          if (!HasLineOfSight(ax, ay)) { ax = tx; ay = ty; }
        }
        dx = ax - mx; dy = ay - my;
        double angle = atan2(dy, dx);

        // Trace bullet path to check if it hits a wall before the enemy
        bool wouldHitWall = false;
        {
          double cx = mx, cy = my;
          double ca = cos(angle), sa = sin(angle);
          for (int s = 0; s < 200; s++) {
            cx += SPEED * ca;
            cy += SPEED * sa;
            int gi = (int)cx, gj = (int)cy;
            if (gi < 0 || gi >= MSZ || gj < 0 || gj >= MSZ) { wouldHitWall = true; break; }
            double de = (cx - tx)*(cx - tx) + (cy - ty)*(cy - ty);
            if (de < 4.0) break;
            if (map[gi][gj] == WALL || map[gi][gj] == STONE) { wouldHitWall = true; break; }
          }
        }

        if (wouldHitWall) {
          consecutiveBlockedShots++;
          if (consecutiveBlockedShots > 4) {
            double perpX = -dy / dist, perpY = dx / dist;
            double stepSize = SPEED * 6.0;
            int side = (consecutiveBlockedShots % 2 == 0) ? 1 : -1;
            double nx = x + side * perpX * stepSize;
            double ny = y + side * perpY * stepSize;
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
              x = nx; y = ny;
            }
          }
        } else {
          consecutiveBlockedShots = 0;
          if (fireCooldown <= 0) {
            int slot = -1;
            for (int s = 0; s < MAX_BULLETS; s++) {
              if (bullets[s] == nullptr) { slot = s; break; }
            }
            if (slot >= 0) {
              setAmmo(ammo - 0.01);
              bullets[slot] = new Bullet(x + 1.5, y + 1.5, angle, team);
              bullets[slot]->setIsMoving(true);
              fireCooldown = FIRE_COOLDOWN_FRAMES;
              shotThisFrame = true;
            }
          }
        }
      }
    }

    if (fireCooldown > 0) fireCooldown--;

    // Track frames without shooting to break combat stalls (LOS blocked by obstacle)
    bool anyBulletAlive = false;
    for (int s = 0; s < MAX_BULLETS; s++)
      if (bullets[s] != nullptr) { anyBulletAlive = true; break; }
    if (shotThisFrame || anyBulletAlive || pGrenade != nullptr) {
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
        double gx = ex + 1.5, gy = ey + 1.5;
        double dx = gx - (x + 1.5), dy = gy - (y + 1.5);
        double dist = sqrt(dx*dx + dy*dy);
        double tgx = gx, tgy = gy;
        if (dist > 0.5) {
          double perpX = -dy / dist, perpY = dx / dist;
          double offset = 2.5 + (rand() % 100) / 50.0;
          int side = (rand() % 2) ? 1 : -1;
          tgx = gx + side * perpX * offset;
          tgy = gy + side * perpY * offset;
        }
        if (HasLineOfSight(tgx, tgy)) {
        pGrenade = new Grenade(x + 1.5, y + 1.5, tgx, tgy, team);
        grenadeThrownThisRound = true;
        if (grenadeMsgCooldown <= 0)
          grenadeMsgCooldown = 300;
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

Bullet **WarriorNPC::getBullets() { return bullets; }
Grenade *WarriorNPC::getGrenade() const { return pGrenade; }
void WarriorNPC::setGrenade(Grenade *grenade) { pGrenade = grenade; }
