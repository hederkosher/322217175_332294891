#include "GoToWarrior.h"
#include "NPC.h"
#include "LogisticNPC.h"
#include "GiveAmmo.h"

void GoToWarrior::OnEnter(NPC* pn)
{
    double x, y;
    if (auto ln = dynamic_cast<LogisticNPC*>(pn)) {
        pn->setIsMoving(true);
        if (ln->getWarriorPointer() && ln->getWarriorPointer()->getHp() > 0
            && ln->getWarriorPointer()->getAmmo() < AMMO_MAX / 2)
        {
            ln->setGoToWarrior(true);
            ln->getWarriorPointer()->getPosition(x, y);
            pn->setTarget(x, y);
            ln->setStayedAtArmory(false);
            ln->PlanPathTo();
            return;
        }
        ln->setStayedAtArmory(true);
    }
}

void GoToWarrior::Transition(NPC* pn)
{
    OnExit(pn);
    if (auto ln = dynamic_cast<LogisticNPC*>(pn))
    {
        pn->setCurrentState(new GiveAmmo());
        pn->getCurrentState()->OnEnter(pn);
        return;
    }
}

void GoToWarrior::OnExit(NPC* pn)
{
    pn->setIsMoving(false);
}
