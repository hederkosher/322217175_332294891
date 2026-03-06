#pragma once
#include "GoToArmory.h"
#include "NPC.h"
#include "WarriorNPC.h"


class SupplyNPC : public NPC {
private:
  double ammo;
  bool isFillingAmmo = false;
  bool isGivingAmmo = false;
  bool goToNeedyWarrior = false;
  WarriorNPC *pWarrior = nullptr;
  bool waitingAtArmory = false;

  int scanCooldown = 0;

public:
  SupplyNPC(double positionX, double positionY, char character, int team,
            int type);
  void setWarriorPointer(WarriorNPC *pW);
  WarriorNPC *getWarriorPointer();
  bool getGoToNeedyWarrior();
  void setGoToNeedyWarrior(bool goToW);
  bool getIsGivingAmmo();
  void setIsGivingAmmo(bool isGive);
  bool getIsFillingAmmo();
  void setIsFillingAmmo(bool isFill);
  void setAmmo(double value);
  bool getWaitingAtArmory();
  void setWaitingAtArmory(bool stayed);
  void DoSomeWork();
  void show() override;

  // Autonomous warrior scanning
  WarriorNPC *FindWarriorNeedingAmmo();
};
