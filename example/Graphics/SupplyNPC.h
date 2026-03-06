#pragma once
#include "GoToArmory.h"
#include "NPC.h"
#include "WarriorNPC.h"


class SupplyNPC : public NPC {
private:
  double ammo;
  bool isFillingAmmo = false;
  bool isGivingAmmo = false;
  bool goToWarrior = false;
  WarriorNPC *pWarrior = nullptr;
  bool stayedAtArmory = false;

  int scanCooldown = 0;
  bool supplyMessageShown = false;

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
};
