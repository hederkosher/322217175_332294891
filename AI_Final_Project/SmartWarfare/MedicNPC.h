#pragma once
#include "NPC.h"
#include "GoToMedicine.h"
#include "WarriorNPC.h"

class MedicNPC : public NPC {
private:
    bool isFillingMedicine = false;
    int fillingDepotIndex = -1;
    double medicine;
    bool isGivingMedicine = false;
    bool stayedAtMedicine = false;

    NPC* pTarget = nullptr;
    bool goToTarget = false;

    int scanCooldown = 0;
    bool fleeMessageShown = false;
    int fleeFrames = 0;
    int pathCooldown = 0;  // avoid repathing every 50 frames (giggle + FPS drop)

public:
    MedicNPC(double positionX, double positionY, char character, int team, int type);
    NPC* getTargetNPC();
    void setTargetNPC(NPC* pT);
    bool getGoToTarget();
    void setGoToTarget(bool goToTarget);

    bool getIsGivingMedicine();
    void setIsGivingMedicine(bool isGive);
    bool getIsFillingMedicine();
    void setIsFillingMedicine(bool isFill);
    int getFillingDepotIndex() const { return fillingDepotIndex; }
    void setFillingDepotIndex(int idx) { fillingDepotIndex = idx; }
    bool getStayedAtMedicine();
    void setStayedAtMedicine(bool stayed);
    void DoSomeWork();
    void setMedicine(double value);
    double getMedicine() const { return medicine; }
    void show() override;

    // Autonomous teammate scanning
    NPC* FindInjuredTeammate();
    // Closest living warrior (for fleeing toward teammates when under fire)
    NPC* FindClosestWarrior();
};
