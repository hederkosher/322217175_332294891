#pragma once
#include "GoToArmory.h"
#include "NPC.h"
#include "WarriorNPC.h"


class SupplyNPC : public NPC {
private:
  double ammo;
  bool isFillingAmmo = false;
  bool isGivingAmmo = false;
  int fillingDepotIndex = -1;
  bool goToWarrior = false;
  WarriorNPC *pWarrior = nullptr;
  bool stayedAtArmory = false;

  int scanCooldown = 0;
  int counter = 0;
  int pathCooldown = 0;  // avoid repathing every 50 frames (giggle + FPS drop)
  int framesStuck = 0;
  int fleeFrames = 0;
  bool supplyMessageShown = false;
  bool fleeMessageShown = false;

public:
  SupplyNPC(double positionX, double positionY, char character, int team,
            int type);
  void setWarriorPointer(WarriorNPC *pW);
  WarriorNPC *getWarriorPointer();
  bool getGoToWarrior();
  void setGoToWarrior(bool goToW);
  bool getIsGivingAmmo();
  void setIsGivingAmmo(bool isGive);
  bool getIsFillingAmmo();
  void setIsFillingAmmo(bool isFill);
  int getFillingDepotIndex() const { return fillingDepotIndex; }
  void setFillingDepotIndex(int idx) { fillingDepotIndex = idx; }
  void setAmmo(double value);
  double getAmmo() const { return ammo; }
  bool getStayedAtArmory();
  void setStayedAtArmory(bool stayed);
  void DoSomeWork();
  void show() override;

  // Autonomous warrior scanning
  WarriorNPC *FindWarriorNeedingAmmo();
  // Warrior with lowest ammo (any living warrior) - for moving toward team to resupply
  WarriorNPC *FindWarriorWithLowestAmmo();
  // Closest living warrior (for fleeing toward teammates when under fire)
  WarriorNPC *FindClosestWarrior();
};
