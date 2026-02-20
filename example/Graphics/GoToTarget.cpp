#include "GoToTarget.h"
void GoToTarget::OnEnter(NPC* pn)
{
    double x, y;
    if (auto mn = dynamic_cast<MedicNPC*>(pn)) {
        pn->setIsMoving(true);
        if (mn->getTargetNPC()->getHp() < MAX_HP / 2 && mn->getTargetNPC()->getHp() > 0)
        {
                mn->setGoToTarget(true);
                mn->getTargetNPC()->getPosition(x, y);
                pn->setTarget(x, y);
                mn->setStayedAtMedicine(false);
                mn->PlanPathTo();//AStar
                return;
        }
        mn->setStayedAtMedicine(true);
    }
    else if (auto cn = dynamic_cast<CommanderNPC*>(pn))
    {
        int randomI;
        if (cn->getTeam() == 2)//blue - need up to 30%
            randomI =  10 + rand() % 20;
        else
			randomI = 80 + rand() % 41;
        pn->setIsMoving(true);
        cn->setGoToTarget(true);
        cn->setIsEscaping(true);
		int spreadRadius = 20;
        int offsetX = (rand() % (2 * spreadRadius + 1)) - spreadRadius;
        int offsetY = (rand() % (2 * spreadRadius + 1)) - spreadRadius;
        int randomJ = rand() % MSZ;
        int targetI = randomI + offsetX;
        int targetJ = randomJ + offsetY;
        if (targetI < 0) targetI = 0;
        if (targetI >= MSZ) targetI = MSZ - 1;
        if (targetJ < 0) targetJ = 0;
        if (targetJ >= MSZ) targetJ = MSZ - 1;
		cn->setTarget(targetI, targetJ); 
        cn->PlanPathTo();//AStar
        return;
    }
}

void GoToTarget::Transition(NPC* pn)
{
    OnExit(pn); 
    if (auto mn = dynamic_cast<MedicNPC*>(pn)) {
        pn->setCurrentState(new GiveMedicine());
        pn->getCurrentState()->OnEnter(pn);
        return;
    }
}

void GoToTarget::OnExit(NPC* pn)
{
    pn->setIsMoving(false);
    if (auto cn = dynamic_cast<CommanderNPC*>(pn))
    {
        cn->setGoToTarget(false);
        cn->setIsEscaping(false);
    }
}