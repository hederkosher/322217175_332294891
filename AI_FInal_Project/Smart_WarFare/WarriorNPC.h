#pragma once
#include "NPC.h"
#include "Grenade.h"

class Bullet;

class WarriorNPC : public NPC {
private:
    double ammo;
    bool isAttacking;
    Bullet* pBullet = nullptr;
    Grenade* pGrenade = nullptr;
    bool arrivedAtTarget = false;
  bool hasThrownGrenade = false;

  // Message throttling flags
  bool printedHpFleeMsg = false;
  bool printedLowAmmoMsg = false;
  bool printedAttackMsg = false;

    // Personality-derived thresholds
    double hpFleeThreshold;
    double ammoFleeThreshold;

    // Search behavior
    int searchTargetRoom = -1;
    int framesAtTarget = 0;

    // Grenade attack cooldown
    int grenadeCounter = 0;

    // Flee repath counter (replan path to medic/supply periodically)
    int fleeRepathCounter = 0;

    // Counts consecutive shots that were blocked by an obstacle (not enemy hit)
    int blockedShotsCounter = 0;

public:
    WarriorNPC(double positionX, double positionY, char character, int team, int type);
    bool isInRisk() const override;
    void setAmmo(double value);
    double getAmmo();
    void DoSomeWork() override;
    void show() override;
    Bullet* getBullet() const;
    void setBullet(Bullet* bullet);
    Grenade* getGrenade() const;
    void setGrenade(Grenade* grenade);
    void setIsAttacking(bool value);
    bool FindVisibleEnemy(double& outX, double& outY);
    bool getArrivedAtTarget() const { return arrivedAtTarget; }
    void setArrivedAtTarget(bool value) { arrivedAtTarget = value; }

    // Room-based enemy detection
    NPC* FindEnemyInSameRoom();
    void EvaluatePriorities();
    void SearchForEnemies();
};
