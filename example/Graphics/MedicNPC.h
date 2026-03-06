#pragma once
#include "NPC.h"
#include "GoToMedicine.h"
#include "WarriorNPC.h"

class MedicNPC : public NPC {
private:
    bool isFillingMedicine = false;
    double medicine;
    bool isGivingMedicine = false;
    bool waitingAtMedicine = false;

    NPC* pTarget = nullptr;
    bool goToInjured = false;

    int scanCooldown = 0;

public:
    MedicNPC(double positionX, double positionY, char character, int team, int type);
    NPC* getTargetNPC();
    void setTargetNPC(NPC* pT);
    bool getGoToInjured();
    void setGoToInjured(bool goToInjured);

    bool getIsGivingMedicine();
    void setIsGivingMedicine(bool isGive);
    bool getIsFillingMedicine();
    void setIsFillingMedicine(bool isFill);
    bool getWaitingAtMedicine();
    void setWaitingAtMedicine(bool stayed);
    void DoSomeWork();
    void setMedicine(double value);
    void show() override;

    // Autonomous teammate scanning
    NPC* FindInjuredTeammate();
};
