#include "MedicNPC.h"
#include "GoToTarget.h"
#include "GoToDefenseState.h"
#include "IdleState.h"
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

NPC* MedicNPC::FindInjuredTeammate()
{
	if (!myTeam) return nullptr;
	NPC* worst = nullptr;
	double worstHp = MAX_HP;

	for (int i = 0; i < TEAM_SIZE; i++) {
		if (myTeam[i] && myTeam[i] != this && myTeam[i]->getHp() > 0
			&& myTeam[i]->getHp() < MAX_HP * 0.7)
		{
			if (myTeam[i]->getHp() < worstHp) {
				worstHp = myTeam[i]->getHp();
				worst = myTeam[i];
			}
		}
	}
	return worst;
}

void MedicNPC::DoSomeWork()
{
	// Priority 0: Self-preservation -- flee when HP is critically low
	double fleeThreshold = MAX_HP * 0.4;
	if (hp < fleeThreshold && hp > 0) {
		if (!isFleeing) {
			isFleeing = true;
			std::string color = (team == 1 ? TEAM1 : TEAM2);
			std::cout << color << "Medic team " << team
				<< ": HP critical (" << (int)hp << "), retreating!" << RESET << std::endl;

			// Interrupt current activity
			isGivingMedicine = false;
			isFillingMedicine = false;
			stayedAtMedicine = false;
			goToTarget = false;
			if (pTarget) {
				pTarget->setIsGettingHp(false);
				pTarget = nullptr;
			}

			if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
			pCurrentState = new GoToDefenseState();
			pCurrentState->OnEnter(this);
		}

		// Self-heal while in cover
		if ((dynamic_cast<GoToDefenseState*>(pCurrentState) ||
		     dynamic_cast<IdleState*>(pCurrentState)) && medicine > 0) {
			hp += 0.5;
			medicine -= 0.1;
			if (hp > MAX_HP) hp = MAX_HP;
			if (medicine < 0) medicine = 0;
		}

		// Still follow path if moving to cover — only "arrive" when in a room
		if (isMoving && FollowPlannedPath(1)) {
			if (!IsInCorridor()) {
				if (pCurrentState)
					pCurrentState->Transition(this);
			} else {
				MoveToNearestRoom();
			}
		}
		return;
	}

	// Recovery: HP back above threshold
	if (isFleeing) {
		isFleeing = false;
		std::string color = (team == 1 ? TEAM1 : TEAM2);
		std::cout << color << "Medic team " << team
			<< ": HP recovered, resuming duties." << RESET << std::endl;
		if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
		pCurrentState = new GoToMedicine();
		pCurrentState->OnEnter(this);
	}

	// Autonomous: if no target and have medicine, scan teammates
	scanCooldown--;
	if (!pTarget && medicine >= MEDICINE_MAX * 0.3 && !isFillingMedicine && scanCooldown <= 0)
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
		scanCooldown = 30;
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
			// Transition when within healing range (fixes chase deadlock with moving Supply)
			if (Distance(x, y, pos_x, pos_y) <= HEAL_RANGE) {
				if (pCurrentState)
					pCurrentState->Transition(this);
			} else if (FollowPlannedPath(1)) {
				if (!IsInCorridor()) {
					if (pCurrentState)
						pCurrentState->Transition(this);
				} else {
					MoveToNearestRoom();
				}
			}
		} else {
			// Only treat path complete when in a room (never stop in corridor)
			if (FollowPlannedPath(1)) {
				if (!IsInCorridor()) {
					if (pCurrentState)
						pCurrentState->Transition(this);
				} else {
					MoveToNearestRoom();
				}
			}
		}
	}

	if (isFillingMedicine)
	{
		if (medicine < MEDICINE_MAX)
		{
			medicine += 0.1;
		}
		else if (pTarget)
		{
			if (pTarget->getHp() < MAX_HP && pTarget->getHp() > 0)
				pCurrentState->Transition(this);
		}
		else
		{
			// Medicine full but no target -- exit FillMedicine so scan code can run
			if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
			pCurrentState = nullptr;
			isFillingMedicine = false;
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

	// Follow nearest warrior when idle (not moving, not filling, not giving, has medicine)
	if (!isMoving && !isFillingMedicine && !isGivingMedicine && !goToTarget
		&& medicine >= MEDICINE_MAX * 0.3 && myTeam)
	{
		followCooldown--;
		int cooldownLimit = IsInCorridor() ? 0 : 60;
		if (followCooldown <= 0)
		{
			NPC* nearest = nullptr;
			double bestDist = 99999.0;
			for (int i = 0; i < 2; i++) {
				if (myTeam[i] && myTeam[i]->getHp() > 0) {
					double wx, wy;
					myTeam[i]->getPosition(wx, wy);
					double d = Distance(x, y, wx, wy);
					if (d < bestDist) {
						bestDist = d;
						nearest = myTeam[i];
					}
				}
			}
			if (nearest && bestDist > 8.0) {
				double wx, wy;
				nearest->getPosition(wx, wy);
				setTarget(wx, wy);
				PlanPathTo();
				isMoving = true;
			} else if (IsInCorridor()) {
				MoveToNearestRoom();
			}
			followCooldown = cooldownLimit;
		}
	}

	// Catch-all: never idle in a corridor
	if (!isMoving && IsInCorridor()) {
		MoveToNearestRoom();
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

	if (na < 0.20)        glColor3d(1.0, 0.1, 0.1);      // critical: red
	else if (na < 0.60)   glColor3d(0.9, 0.7, 0.0);      // low: yellow
	else                  glColor3d(0.0, 0.9, 0.7);       // good: cyan-green

	double fillH = barH * na;
	glBegin(GL_QUADS);
	glVertex2d(barX0, barY0);
	glVertex2d(barX0 + barW, barY0);
	glVertex2d(barX0 + barW, barY0 + fillH);
	glVertex2d(barX0, barY0 + fillH);
	glEnd();
}
