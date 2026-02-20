#pragma once
#include "NPC.h"
#include "GoToMedicine.h"
#include "WarriorNPC.h"

static int medic_counter = 0;

class MedicNPC : public NPC {
private:
    bool isFillingMedicine = false;
    double medicine;
    bool isGivingMedicine = false;
	bool stayedAtMedicine = false;

    NPC* pTarget = nullptr;
    bool goToTarget = false;

public:
    MedicNPC(double positionX, double positionY, char character, int team , int type);
    //pTarget
	NPC* getTargetNPC();
    void setTargetNPC(NPC* pT);
	//goToTarget
	bool getGoToTarget();
    void setGoToTarget(bool goToTarget);


	bool getIsGivingMedicine();
	void setIsGivingMedicine(bool isGive);
    bool getIsFillingMedicine();
    void setIsFillingMedicine(bool isFill);
	bool getStayedAtMedicine();
	void setStayedAtMedicine(bool stayed);
    void DoSomeWork();
    void setMedicine(double value);
    void show() override;
};