#include "IdleState.h"
#include "NPC.h"
#include "LogisticNPC.h"

void IdleState::OnEnter(NPC* pn)
{
    pn->setIsMoving(false);
}

void IdleState::Transition(NPC* pn)
{
    OnExit(pn);
}

void IdleState::OnExit(NPC* pn)
{
    if (auto Logistic = dynamic_cast<LogisticNPC*>(pn))
    {
        pn->setIsMoving(true);
    }
}
