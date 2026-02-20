#pragma once
#include "NPC.h"
#include "IdleState.h"
#include "AttackState.h"

class CommanderNPC : public NPC {
    NPC** teamNPC;
	bool isOrderingDefend = false;
	bool isOrderingAttack = false;
	bool isOrderingSearch = false;
    bool isEscaping = false;
	bool goToTarget = false;
public:
    CommanderNPC(double positionX, double positionY, char character, int team , int type, NPC** teamNPC);
    void GiveTargetToWarrior(int warriorNumber, int i, int j);
    void orderTeamToSearch();
	void orderTeamToAttack();
    void orderTeamToDefend();
    bool isTeamInRisk() const;
    void DoSomeWork();
    void show() override;
    bool getIsOrderingDefend() { return isOrderingDefend; }
	void setIsOrderingDefend(bool orderingDefend) { isOrderingDefend = orderingDefend; }
	bool getIsOrderingAttack() { return isOrderingAttack; }
	void setIsOrderingAttack(bool orderingAttack) { isOrderingAttack = orderingAttack; }
	bool getIsEscaping() { return isEscaping; }
	void setIsEscaping(bool escaping) { isEscaping = escaping; }
	bool getGoToTarget() { return goToTarget; }
	void setGoToTarget(bool goToTarget) { this->goToTarget = goToTarget; }
    void UpdateVisibility(NPC* myTeam[TEAM_SIZE], NPC* enemyTeam[TEAM_SIZE]) override;
    void DrawVisibilityMap() const override;
};