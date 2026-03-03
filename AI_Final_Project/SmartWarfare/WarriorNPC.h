#pragma once
#include "NPC.h"
#include "Grenade.h"

class Bullet;

const int MAX_BULLETS = 10;
const int FIRE_COOLDOWN_FRAMES = 72;   // 

class WarriorNPC : public NPC {
private:
    double ammo;
    bool isAttacking;
    Bullet* bullets[MAX_BULLETS] = {};
    int fireCooldown = 0;
    Grenade* pGrenade = nullptr;
    bool arrivedAtTarget = false;
    bool grenadeThrownThisRound = false;  // each warrior can throw only 1 grenade per round

    // Personality-derived thresholds
    double hpFleeThreshold;
    double ammoFleeThreshold;

    // Search behavior
    int searchTargetRoom = -1;
    int framesAtTarget = 0;
    int searchCooldown = 0;  // throttle SearchForEnemies when path fails

    // Chase logic: when enemy flees, chase toward last known position
    double lastKnownEnemyX = -1;
    double lastKnownEnemyY = -1;

    // One-time console messages until HP/ammo restored
    bool lowHpMessageShown = false;
    bool lowAmmoMessageShown = false;
    int attackMsgCooldown = 0;
    int grenadeMsgCooldown = 0;

    // Combat: detect when warrior can't shoot (LOS blocked) and needs to reposition
    int framesWithoutShooting = 0;
    int consecutiveBlockedShots = 0;

    // Per-warrior medic repath timer (was incorrectly static/shared)
    int medicRepathFrames = 0;
    // When pathfinding to medic fails repeatedly, give up for a while and fight instead
    int medicGiveUpFrames = 0;
    // Grenade evasion: flee from nearby enemy grenades
    bool fleeingGrenade = false;

    // Detect when warrior is stuck (not moving)
    int framesStuck = 0;

public:
    WarriorNPC(double positionX, double positionY, char character, int team, int type);
    bool isInRisk() const override;
    void setAmmo(double value);
    double getAmmo();
    void DoSomeWork() override;
    void show() override;
    Bullet** getBullets();
    int getMaxBullets() const { return MAX_BULLETS; }
    Grenade* getGrenade() const;
    void setGrenade(Grenade* grenade);
    bool getGrenadeThrownThisRound() const { return grenadeThrownThisRound; }
    void setGrenadeThrownThisRound(bool value) { grenadeThrownThisRound = value; }
    void setIsAttacking(bool value);
    bool FindVisibleEnemy(double& outX, double& outY);
    bool getArrivedAtTarget() const { return arrivedAtTarget; }
    void setArrivedAtTarget(bool value) { arrivedAtTarget = value; }

    // Room-based enemy detection
    NPC* FindEnemyInSameRoom();
    int CountEnemiesInSameRoom();  // number of living enemies in current room

    // Chase logic: last known enemy position when enemy fled
    double getLastKnownEnemyX() const { return lastKnownEnemyX; }
    double getLastKnownEnemyY() const { return lastKnownEnemyY; }
    void EvaluatePriorities();
    void SearchForEnemies();
};
