#include "IdleState.h"
#include "NPC.h"

void IdleState::OnEnter(NPC* pn)
{
    pn->setIsMoving(false); // Ensure NPC stops
}

void IdleState::Transition(NPC* pn)
{
    OnExit(pn);
}

void IdleState::OnExit(NPC* pn)
{
    if(auto Logistic = dynamic_cast<LogisticNPC*>(pn))
    {
        pn->setIsMoving(true);
	}
}