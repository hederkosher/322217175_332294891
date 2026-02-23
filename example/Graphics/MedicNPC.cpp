#include "MedicNPC.h"
#include "GoToTarget.h"
#include "GoToDefenseState.h"
#include "GoToMedicine.h"
#include "Map.h"
#include <iostream>

static int medic_counter = 0;

MedicNPC::MedicNPC(double positionX, double positionY, char character, int team, int type)
	: NPC(positionX, positionY, character, team, type)
{
	setCurrentState(new GoToMedicine());
	getCurrentState()->OnEnter(this);
	setMedicine(MEDICINE_MAX / 10);
}

NPC* MedicNPC::getTargetNPC() { return pTarget; }
void MedicNPC::setTargetNPC(NPC* pT) { pTarget = pT; }
bool MedicNPC::getGoToTarget() { return goToTarget; }
void MedicNPC::setGoToTarget(bool gt) { this->goToTarget = gt; }

bool MedicNPC::getIsGivingMedicine() { return isGivingMedicine; }
void MedicNPC::setIsGivingMedicine(bool isGive) { isGivingMedicine = isGive; }
bool MedicNPC::getIsFillingMedicine() { return isFillingMedicine; }
void MedicNPC::setIsFillingMedicine(bool isFill) { isFillingMedicine = isFill; }
bool MedicNPC::getStayedAtMedicine() { return stayedAtMedicine; }
void MedicNPC::setStayedAtMedicine(bool stayed) { stayedAtMedicine = stayed; }

// Find an injured teammate (any HP < MAX_HP). Prefer most injured; prefer those not in rooms with enemies (avoid fights).
NPC* MedicNPC::FindInjuredTeammate()
{
	if (!myTeam) return nullptr;
	NPC* bestSafe = nullptr;   // most injured in a room with no enemies
	NPC* bestAny = nullptr;   // most injured in any room
	double worstHpSafe = MAX_HP;
	double worstHpAny = MAX_HP;

	for (int i = 0; i < TEAM_SIZE; i++) {
		NPC* t = myTeam[i];
		if (!t || t == this || t->getHp() <= 0 || t->getHp() >= MAX_HP)
			continue;
		double hp = t->getHp();
		int room = t->getCurrentRoom();
		bool roomHasEnemies = RoomHasEnemies(room, enemyTeam);

		if (hp < worstHpAny) {
			worstHpAny = hp;
			bestAny = t;
		}
		if (!roomHasEnemies && hp < worstHpSafe) {
			worstHpSafe = hp;
			bestSafe = t;
		}
	}
	// Prefer injured in safe rooms; if none, go to most injured even in combat zone
	return bestSafe ? bestSafe : bestAny;
}

void MedicNPC::DoSomeWork()
{
	// When being shot (low HP), flee to cover and search for safety first
	if (isInRisk() && !dynamic_cast<GoToDefenseState*>(pCurrentState)) {
		if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
		pCurrentState = new GoToDefenseState();
		pCurrentState->OnEnter(this);
		std::string color = (team == 1 ? TEAM1 : TEAM2);
		std::cout << color << "Medic team " << team << ": under fire, fleeing to cover!" << RESET << std::endl;
		return;
	}

	// If we have a target that's dead or fully healed, clear and go refill so we don't stand still
	if (pTarget && (pTarget->getHp() <= 0 || pTarget->getHp() >= MAX_HP)) {
		pTarget = nullptr;
		setGoToTarget(false);
		if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
		pCurrentState = new GoToMedicine();
		pCurrentState->OnEnter(this);
		return;
	}

	// Autonomous: search for injured teammates (including critical HP). Scan regularly.
	scanCooldown--;
	if (!pTarget && medicine >= MEDICINE_MAX * 0.2 && !isFillingMedicine && scanCooldown <= 0)
	{
		NPC* injured = FindInjuredTeammate();
		if (injured) {
			pTarget = injured;
			std::string color = (team == 1 ? TEAM1 : TEAM2);
			std::cout << color << "Medic team " << team
				<< ": detected injured teammate, going to heal!" << RESET << std::endl;

			if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
			pCurrentState = new GoToTarget();
			pCurrentState->OnEnter(this);
		}
		scanCooldown = 120;
	}

	if (isMoving)
	{
		double pos_x, pos_y;
		if (goToTarget && pTarget)
		{
			pTarget->getPosition(pos_x, pos_y);
			setTarget(pos_x, pos_y);
			if (medic_counter % 50 == 0)
				PlanPathTo();
			medic_counter++;
		}
		if (FollowPlannedPath(1))
		{
			pCurrentState->Transition(this);
		}
	}

	if (isFillingMedicine)
	{
		if (medicine < MEDICINE_MAX)
		{
			medicine += 0.1;
		}
		else
		{
			// Done filling: always leave FillMedicine and go search for soldiers to help (GoToTarget)
			pCurrentState->Transition(this);
		}
	}

	if (stayedAtMedicine)
	{
		if (pTarget && pTarget->getHp() < MAX_HP && pTarget->getHp() > 0)
		{
			if (medicine >= MEDICINE_MAX)
				if (auto GoToTargetState = dynamic_cast<GoToTarget*>(pCurrentState))
					GoToTargetState->OnEnter(this);
		}
	}

	if (isGivingMedicine)
	{
		if (goToTarget && pTarget && pTarget->getHp() < MAX_HP && pTarget->getHp() > 0)
		{
			pTarget->setHp(pTarget->getHp() + 0.5);
			medicine -= 0.1;
			if (medicine <= 0)
				pCurrentState->Transition(this);
		}
		else
		{
			pCurrentState->Transition(this);
		}
	}
}

void MedicNPC::setMedicine(double value) { medicine = value; }

void MedicNPC::show() {
	NPC::show();
	double size = 3.0;

	const double barW = 0.35;
	const double barH = size;
	const double barX0 = x - 0.6;
	const double barY0 = y;

	double na = medicine / MEDICINE_MAX;
	if (na < 0.0) na = 0.0;
	if (na > 1.0) na = 1.0;

	if (na < 0.20)        glColor3d(0.8, 0.1, 0.1);
	else if (na < 0.60)   glColor3d(0.5, 0.8, 0.2);
	else                  glColor3d(0.0, 0.9, 0.7);

	double fillH = barH * na;
	glBegin(GL_QUADS);
	glVertex2d(barX0, barY0);
	glVertex2d(barX0 + barW, barY0);
	glVertex2d(barX0 + barW, barY0 + fillH);
	glVertex2d(barX0, barY0 + fillH);
	glEnd();
}
