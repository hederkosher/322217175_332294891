#include "GoToDefenseState.h"
#include "BFS.h"
#include "IdleState.h"
#include "NPC.h"

void GoToDefenseState::OnEnter(NPC* pn)
{
    double currentX, currentY;
    pn->getPosition(currentX, currentY);

    std::vector<std::pair<int, int>> path;

    if (BFS::FindShortestPathToCover(currentX, currentY, pn->getTeam(), path) && !path.empty()) {
        pn->setPath(path);
        pn->setIsMoving(true);
    }
    else {
        pn->setIsMoving(false);
    }
}

void GoToDefenseState::Transition(NPC* pn)
{
    if (pn->getIsMoving() && pn->getPathIndex() == -1)
    {
        OnExit(pn);
        pn->setCurrentState(new IdleState());
        pn->getCurrentState()->OnEnter(pn);
    }
}

void GoToDefenseState::OnExit(NPC* pn)
{
    pn->setIsMoving(false);
    pn->setPath({});
}
