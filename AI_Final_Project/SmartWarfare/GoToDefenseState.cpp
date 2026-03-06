#include "GoToDefenseState.h"
#include "BFS.h"
#include "IdleState.h"
#include "MoveToTargetState.h"
#include "NPC.h"
#include "WarriorNPC.h"
#include "Definitions.h"

void GoToDefenseState::OnEnter(NPC* pn)
{
    double currentX, currentY;
    pn->getPosition(currentX, currentY);
    std::vector<std::pair<int, int>> path;
    if (BFS::FindShortestPathToCover(currentX, currentY, pn->getTeam(), path) && !path.empty()) {
        pn->setPath(path);
        pn->setIsMoving(true);
    } else {
        pn->setIsMoving(false);
    }
}

void GoToDefenseState::Transition(NPC* pn)
{
    auto* warrior = dynamic_cast<WarriorNPC*>(pn);
    if (!warrior) return;

    double hpRatio = warrior->getHp() / MAX_HP;
    bool ammoOk = warrior->getAmmo() >= AMMO_MAX * AMMO_NEED_RESUPPLY_RATIO;

    // At cover (arrived): enter IdleState for explicit "hold at cover"
    if (pn->getIsMoving() && pn->getPathIndex() == -1) {
        pn->setIsMoving(false);
        pn->setPath({});
        pn->setCurrentState(new IdleState());
        pn->getCurrentState()->OnEnter(pn);
        return;
    }

    // HP recovered and ammo ok -> back to fight/search
    if (hpRatio >= HP_RECOVER_RATIO && ammoOk) {
        OnExit(pn);
        pn->setCurrentState(new MoveToTargetState());
        pn->getCurrentState()->OnEnter(pn);
        return;
    }
    // Medic reachable and HP still low -> go to medic
    NPC** myTeam = warrior->getMyTeam();
    if (myTeam && myTeam[2] && myTeam[2]->getHp() > 0 && hpRatio < HP_NEED_HEAL_RATIO) {
        OnExit(pn);
        double mx, my;
        myTeam[2]->getPosition(mx, my);
        pn->setTarget(mx, my);
        warrior->PlanPathTo();
        pn->setCurrentState(new MoveToTargetState());
        pn->getCurrentState()->OnEnter(pn);
    }
}

void GoToDefenseState::OnExit(NPC* pn)
{
    pn->setIsMoving(false);
    pn->setPath({});
}
