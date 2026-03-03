#include "MedicNPC.h"
#include "GoToTarget.h"
#include "GoToMedicine.h"
#include "FillMedicine.h"
#include "GiveMedicine.h"
#include "Map.h"

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

NPC* MedicNPC::FindClosestWarrior() {
	if (!myTeam) return nullptr;
	NPC* closest = nullptr;
	double bestDist = 1e9;
	for (int i = 0; i < 2; i++) {
		NPC* t = myTeam[i];
		if (!t || t->getHp() <= 0) continue;
		double tx, ty;
		t->getPosition(tx, ty);
		double dx = tx - x, dy = ty - y;
		double d = dx * dx + dy * dy;
		if (d < bestDist) { bestDist = d; closest = t; }
	}
	return closest;
}

void MedicNPC::DoSomeWork()
{
	// When being shot (low HP) or when seeing an enemy nearby, run toward closest warrior to avoid them
	bool shouldAvoid = isInRisk() || HasVisibleEnemyWithin(SUPPORT_AVOID_ENEMY_DIST);
	if (shouldAvoid && fleeFrames < 480) {
		fleeFrames++;
		NPC* closest = FindClosestWarrior();
		if (closest) {
			pTarget = closest;
			setGoToTarget(true);
			if (!dynamic_cast<GoToTarget*>(pCurrentState)) {
				if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
				pCurrentState = new GoToTarget();
				pCurrentState->OnEnter(this);
				if (!fleeMessageShown)
					fleeMessageShown = true;
			}
			double wx, wy;
			closest->getPosition(wx, wy);
			setTarget(wx, wy);
			isMoving = true;
			if (path.empty() || pathIndex < 0 || pathCooldown <= 0) {
				if (PlanPathTo()) pathCooldown = 45;
				else pathCooldown = 20;  // failed (e.g. budget): don't retry every frame
			}
			if (pathCooldown > 0) pathCooldown--;
			FollowPlannedPath(1);
		}
		return;
	}
	if (!shouldAvoid) fleeFrames = 0;
	fleeMessageShown = false;

	// If we have a target that's dead or fully healed, clear and go refill so we don't stand still
	if (pTarget && (pTarget->getHp() <= 0 || pTarget->getHp() >= MAX_HP)) {
		pTarget->setIsGettingHp(false);
		if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
		pTarget = nullptr;
		setGoToTarget(false);
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

			if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
			pCurrentState = new GoToTarget();
			pCurrentState->OnEnter(this);
		}
		scanCooldown = 120;
	}

	if (pathCooldown > 0) pathCooldown--;
	if (isMoving)
	{
		// Going to medicine depot (spawn refill): retry path often if failed so we don't stand still
		if (dynamic_cast<GoToMedicine*>(pCurrentState) && (path.empty() || pathIndex < 0) && pathCooldown <= 0) {
			if (PlanPathTo()) pathCooldown = 45;
			else pathCooldown = 5;
		}
		double pos_x, pos_y;
		if (goToTarget && pTarget)
		{
			pTarget->getPosition(pos_x, pos_y);
			setTarget(pos_x, pos_y);
			if (path.empty() || pathIndex < 0 || pathCooldown <= 0) {
				if (PlanPathTo()) pathCooldown = 45;
				else pathCooldown = 20;  // failed (e.g. budget): don't retry every frame
			}
		}
		if (FollowPlannedPath(1))
		{
			pCurrentState->Transition(this);
		}
	}

	if (isFillingMedicine)
	{
		// Claim depot on first fill frame
		if (fillingDepotIndex < 0) {
			double bestDist = 1e9;
			for (int i = 0; i < numMedicine; i++) {
				double dx = x - medicineX[i];
				double dy = y - medicineY[i];
				double d = dx * dx + dy * dy;
				if (d < bestDist) { bestDist = d; fillingDepotIndex = i; }
			}
			if (fillingDepotIndex >= 0) {
				int occ = medicineOccupiedBy[fillingDepotIndex];
				if (occ != 0 && occ != team) {
					fillingDepotIndex = -1;
					isFillingMedicine = false;
					if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
					pCurrentState = new GoToMedicine();
					pCurrentState->OnEnter(this);
				} else {
					medicineOccupiedBy[fillingDepotIndex] = team;
				}
			}
		}

		if (isFillingMedicine) {
			bool nearMedicine = false;
			for (int i = 0; i < numMedicine; i++) {
				double dx = x - medicineX[i];
				double dy = y - medicineY[i];
				if (dx * dx + dy * dy < 25.0) { nearMedicine = true; break; }
			}
			if (!nearMedicine) {
				if (fillingDepotIndex >= 0) { medicineOccupiedBy[fillingDepotIndex] = 0; fillingDepotIndex = -1; }
				isFillingMedicine = false;
				if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
				pCurrentState = new GoToMedicine();
				pCurrentState->OnEnter(this);
			} else if (medicine < MEDICINE_MAX) {
				medicine += 0.1;
			} else {
				if (fillingDepotIndex >= 0) { medicineOccupiedBy[fillingDepotIndex] = 0; fillingDepotIndex = -1; }
				pCurrentState->Transition(this);
			}
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
		else if (!pTarget && medicine < MEDICINE_MAX * 0.5) {
			stayedAtMedicine = false;
			if (pCurrentState) { pCurrentState->OnExit(this); delete pCurrentState; }
			pCurrentState = new GoToMedicine();
			pCurrentState->OnEnter(this);
		}
	}

	if (isGivingMedicine)
	{
		if (!isMoving && goToTarget && pTarget && pTarget->getHp() < MAX_HP && pTarget->getHp() > 0)
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

	if (na < 0.20)        glColor3d(1.0, 0.1, 0.1);
	else if (na < 0.60)   glColor3d(1.0, 0.8, 0.0);
	else                  glColor3d(0.0, 1.0, 0.4);

	double fillH = barH * na;
	glBegin(GL_QUADS);
	glVertex2d(barX0, barY0);
	glVertex2d(barX0 + barW, barY0);
	glVertex2d(barX0 + barW, barY0 + fillH);
	glVertex2d(barX0, barY0 + fillH);
	glEnd();
}
