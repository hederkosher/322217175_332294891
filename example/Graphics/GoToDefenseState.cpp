#include "GoToDefenseState.h"
#include "BFS.h"
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
    // When we arrive at cover, stay in this state (hold at cover) instead of going Idle.
    // Otherwise we'd go Idle -> next frame EvaluatePriorities sees critical HP and
    // switches back to FLEEING -> infinite loop of FLEEING / searching room.
    if (pn->getIsMoving() && pn->getPathIndex() == -1)
    {
        pn->setIsMoving(false);
        pn->setPath({});
        // Stay in GoToDefenseState so we don't trigger re-flee or search logic
    }
}

void GoToDefenseState::OnExit(NPC* pn)
{
    pn->setIsMoving(false);
    pn->setPath({});
}
