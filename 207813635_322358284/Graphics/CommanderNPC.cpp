#include "CommanderNPC.h"
#include "Map.h"
#include "GoToDefenseState.h"
#include <iostream>
using namespace std;


const int ATTACK_SPREAD_RADIUS = 10;

CommanderNPC::CommanderNPC(double positionX, double positionY, char character, int team , int type ,NPC** teamNPC)
	: NPC(positionX, positionY, character, team , type)
{
    hp = MAX_HP / 4;
	this->teamNPC = teamNPC;
}

void CommanderNPC::GiveTargetToWarrior(int warriorNumber,int i , int j)
{
    if (auto w = dynamic_cast<WarriorNPC*>(teamNPC[warriorNumber])) {
        int targetI = i;
        int targetJ = j;
        w->setTarget(targetI, targetJ);
        w->PlanPathTo();
        if (auto idleState = dynamic_cast<IdleState*>(w->getCurrentState())) {
            //attack is in idle state, order to go to attack
            if (w->getCurrentState() != nullptr) {
                w->getCurrentState()->OnExit(w);
            }
            w->setCurrentState(new AttackState());
            w->getCurrentState()->OnEnter(w);
        }
    }
}


void CommanderNPC::orderTeamToSearch()
{
    string color = (team == 1 ? TEAM1 : TEAM2);
    int randomNum;
    cout << color << "Commander" << team << ": No enemies in sight. lets find them!" << RESET << endl;
    int offsetX = (rand() % (2 * ATTACK_SPREAD_RADIUS + 1)) - ATTACK_SPREAD_RADIUS;
    int offsetY = (rand() % (2 * ATTACK_SPREAD_RADIUS + 1)) - ATTACK_SPREAD_RADIUS;

    if (teamNPC[1] != nullptr)
    {
        randomNum = std::rand() % 31;
        GiveTargetToWarrior(1, randomNum + offsetX, randomNum + offsetY);
        cout << color << "Commander " << team << " ordering warrior to search at (" << randomNum + offsetX
            << ", " << randomNum + offsetY << ")" << RESET << endl;
        cout << color << "-------------------------------------------------------------" << RESET << endl;
    }
    if (teamNPC[2] != nullptr)
    {
        randomNum = 60 + (std::rand() % 31);
        GiveTargetToWarrior(2, randomNum + offsetX, randomNum + offsetY);
        cout << color << "Commander " << team << " ordering warrior to search at (" << randomNum + offsetX
            << ", " << randomNum + offsetY << ")" << RESET << endl;
        cout << color << "-------------------------------------------------------------" << RESET << endl;
    }
}

static int commander_counter = 0;

void CommanderNPC::orderTeamToAttack()
{
    string color = (team == 1 ? TEAM1 : TEAM2);
    vector<pair<int, int>> enemyPositions;

    // scan the visibility map to find all visible enemies
    for (int i = 0; i < MSZ; i++) {
        for (int j = 0; j < MSZ; j++) {
            // A positive value in the visibility map indicates an enemy
            if (visibilityMap[i][j] > 0) {
                enemyPositions.push_back({ i, j });
            }
        }
    }
    cout << color << "-------------------------------------------------------------" << RESET << endl;
    // If no enemies are visible, do nothing
    if (enemyPositions.empty()) {
        orderTeamToSearch();
        return;
    }
    // calculate the "center of mass" of the enemies
    int total_i = 0;
    int total_j = 0;
    for (const auto& pos : enemyPositions) {
        total_i += pos.first;
        total_j += pos.second;
    }
    pair<int, int> centralAttackPoint = {
        total_i / enemyPositions.size(),
        total_j / enemyPositions.size()
    };

    cout << color << "Commander " << team <<  " identified enemy center of mass at (" << centralAttackPoint.first
        << ", " << centralAttackPoint.second << ")" << RESET << endl;
    for (int i = 1; i < 3; i++)
    {
        int offsetX = (rand() % (2 * ATTACK_SPREAD_RADIUS + 1)) - ATTACK_SPREAD_RADIUS;
        int offsetY = (rand() % (2 * ATTACK_SPREAD_RADIUS + 1)) - ATTACK_SPREAD_RADIUS;

        int targetI = centralAttackPoint.first + offsetX;
        int targetJ = centralAttackPoint.second + offsetY;

        // Clamp coordinates to map bounds
        if (targetI < 0) targetI = 0;
        if (targetI >= MSZ) targetI = MSZ - 1;
        if (targetJ < 0) targetJ = 0;
        if (targetJ >= MSZ) targetJ = MSZ - 1;

        // If the random spot is unwalkable, use the central point instead
        if (map[targetI][targetJ] != 0) {
            targetI = centralAttackPoint.first;
            targetJ = centralAttackPoint.second;
        }
        if (teamNPC[i] != nullptr)
        {
            GiveTargetToWarrior(i, targetI, targetJ);
            cout << color << "Commander# " << team << " ordering warrior to attack at (" << targetI
                << ", " << targetJ << ")"  << RESET << endl;
        }
    }

   
}

void CommanderNPC::orderTeamToDefend()
{
	for (int i = 0; i < 3; i++) //only order warriors
    {
        NPC* teammate = teamNPC[i];
        if (teammate != nullptr)
        {
            // Clean up the previous state to prevent memory leaks
            if (teammate->getCurrentState() != nullptr) {
                teammate->getCurrentState()->OnExit(teammate);
                delete teammate->getCurrentState();
            }
            // Set the new state
            teammate->setCurrentState(new GoToDefenseState());
            teammate->getCurrentState()->OnEnter(teammate);
        }
    }
}

bool CommanderNPC::isTeamInRisk() const
{
    if (teamNPC[3] == nullptr) return false;
    int count = 0;
    if(teamNPC[1] != nullptr)
        if(auto warrior1 = dynamic_cast<WarriorNPC*>(teamNPC[1])) {
            if (warrior1->isInRisk()) {
                return true;
            }
	    }
    if (teamNPC[2] != nullptr)
        if (auto warrior2 = dynamic_cast<WarriorNPC*>(teamNPC[2])) {
            if (warrior2->isInRisk()) {
                return true;
            }
        }

    return false;
}

static int count_frames = 0;

void CommanderNPC::DoSomeWork()
{
    string color = (team == 1 ? TEAM1 : TEAM2);

    if (hp > 0 && !isGettingHp && hp < MAX_HP / 3 && !isEscaping)
    {
        setCurrentState(new GoToTarget());
		getCurrentState()->OnEnter(this);
    }

    if (isMoving) {
        FollowPlannedPath(0.5);
    }

    if (isTeamInRisk())
    {
        if (!isOrderingDefend)
        {
            isOrderingDefend = true;
            isOrderingAttack = false;
            cout << color << "-------------------------------------------------------------" << RESET << endl;
            cout << color << "Commander detects risk! Ordering team " << team << " to DEFEND!" << RESET << endl;
            cout << color << "-------------------------------------------------------------" << RESET << endl;
            orderTeamToDefend();
        }
    }
    else 
    {
        if (!isOrderingAttack && (teamNPC[1]  || teamNPC[2]))
        {
            isOrderingAttack = true;
            isOrderingDefend = false;
            //print dash
            cout << color << "-------------------------------------------------------------" << RESET << endl;
            cout << color << "Team is safe! Commander ordering team " << team << " to ATTACK!" << RESET << endl;
            orderTeamToAttack(); 
        }

    }

    

    for (int k = 1; k < 3; k++) {
        NPC* teammate = teamNPC[k];
        if (teammate == nullptr)
        {
            continue;
        }
        if (auto w = dynamic_cast<WarriorNPC*>(teammate)) {
            if (w->getArrivedAtTarget())
            {
                vector<pair<int, int>> enemyPositions;
                // Scan the visibility map to find all visible enemies
                for (int i = 0; i < MSZ; i++) {
                    for (int j = 0; j < MSZ; j++) {
                        // A positive value in the visibility map indicates an enemy
                        if (visibilityMap[i][j] > 0) {
                            enemyPositions.push_back({ i, j });
                        }
                    }
                }
                if (enemyPositions.empty()) {
                    cout << color << "-------------------------------------------------------------" << RESET << endl;
                    int offsetX = (rand() % (2 * ATTACK_SPREAD_RADIUS + 1)) - ATTACK_SPREAD_RADIUS;
                    int offsetY = (rand() % (2 * ATTACK_SPREAD_RADIUS + 1)) - ATTACK_SPREAD_RADIUS;
                    int randomI = rand() % MSZ;
                    int randomJ = rand() % MSZ;
                    int targetI = randomI + offsetX;
                    int targetJ = randomJ + offsetY;
                    if (targetI < 0) targetI = 0;
                    if (targetI >= MSZ) targetI = MSZ - 1;
                    if (targetJ < 0) targetJ = 0;
                    if (targetJ >= MSZ) targetJ = MSZ - 1;
                    GiveTargetToWarrior(k, targetI, targetJ);
                    cout << color << "Commander " << team
                        << " ordering warrior to search at ("
                        << targetI << ", " << targetJ << ")"
                        << RESET << endl;
                }
              
            }
		}
    }

	//tell logistic to refill ammo if any teammate ammo < 30%
    for (int k = 1; k < 3; k++) {
        NPC* teammate = teamNPC[k];
        if (teammate == nullptr )
        {
            continue;
        }
        if (auto w = dynamic_cast<WarriorNPC*>(teammate)) {
            if (w->getAmmo() < AMMO_MAX / 3 && w->getHp() > 0) {
                if (auto l = dynamic_cast<LogisticNPC*>(teamNPC[4])) {
                    if (l->getIsGivingAmmo())
                        continue; //skip if logistic is already giving ammo
                    if(l->getWarriorPointer() == w)
                        break; //skip if logistic is already assigned to this warrior
                    l->setWarriorPointer(w);
                    cout << color << "-------------------------------------------------------------" << RESET << endl;
                    cout << color << "Commander " << team << " ordering Logistic to refill ammo for NPC #" << k << RESET << endl;
                    cout << color << "-------------------------------------------------------------" << RESET << endl;
                }
                break; //only refill one NPC at a time
            }
        }
	}
    
  

	//tell medic to heal NPC if any teammate hp < 50%
    for (int k = 0; k < TEAM_SIZE; k++) {
        NPC* teammate = teamNPC[k];
		if (teammate == nullptr || k == 3) //skip medic itself
        {
            continue;
        }
        if (teammate->getHp() < MAX_HP / 2 && teammate->getHp() > 0) {
            if (auto m = dynamic_cast<MedicNPC*>(teamNPC[3])) {
				if (m->getIsGivingMedicine())
                    continue; //skip if medic is already giving medicine
                if(m->getTargetNPC() == teammate)
					break; //skip if medic is already assigned to this teammate
                m->setTargetNPC(teammate);
                cout << color << "-------------------------------------------------------------" << RESET << endl;
				cout << color << "Commander " << team << " ordering Medic to heal NPC #" << k << RESET << endl;
                cout << color << "-------------------------------------------------------------" << RESET << endl;
            }
            break; //only heal one NPC at a time
        }
    }
}

void CommanderNPC::show() {
	NPC::show();
}

void CommanderNPC::UpdateVisibility(NPC* myTeam[TEAM_SIZE], NPC* enemyTeam[TEAM_SIZE])
{
	//clear the visibility map
    for (int i = 0; i < MSZ; i++) {
        for (int j = 0; j < MSZ; j++) {
            this->visibilityMap[i][j] = 0;
        }
    }

 
    for (int k = 0; k < TEAM_SIZE; k++)
    {
        NPC* teammate = teamNPC[k];
        if (teammate == nullptr) continue;


        if (teammate == this) {
            teammate->NPC::UpdateVisibility(myTeam, enemyTeam);
        }
        else {
            teammate->UpdateVisibility(myTeam, enemyTeam);
        }

		//concatenate visibility maps
        int(*teammateMap)[MSZ] = teammate->getVisibilityMap();
        for (int i = 0; i < MSZ; i++) {
            for (int j = 0; j < MSZ; j++) {
                if (teammateMap[i][j] != 0) {
                    this->visibilityMap[i][j] = teammateMap[i][j];
                }
            }
        }
    }
}


void CommanderNPC::DrawVisibilityMap() const
{
    int i, j, k;

    for (i = 0; i < MSZ; i++) {
        for (j = 0; j < MSZ; j++) {
            if (visibilityMap[i][j] > 0) {
				glColor3d(0.6, 0.0, 0.0); //red for enemy
                glBegin(GL_POLYGON);
                glVertex2d(i, j); glVertex2d(i, j + 1); glVertex2d(i + 1, j + 1); glVertex2d(i + 1, j);
                glEnd();
            }

            else if (visibilityMap[i][j] < 0) {
				glColor3d(0.0, 0.5, 1.0); //blue for teammate
                glBegin(GL_POLYGON);
                glVertex2d(i, j); glVertex2d(i, j + 1); glVertex2d(i + 1, j + 1); glVertex2d(i + 1, j);
                glEnd();
            }
			// cell not directly visible by commander
            else {
                bool isVisibleByTeam = false;
                for (k = 0; k < TEAM_SIZE; k++) {
                    NPC* teammate = teamNPC[k];
                    if (teammate != nullptr && teammate->HasLineOfSight(i + 0.5, j + 0.5)) {
                        isVisibleByTeam = true;
                        break;
                    }
                }

                if (isVisibleByTeam) {
                    switch (map[i][j]) {
                    case 0: glColor3d(0.9, 0.9, 0.9); break;
                    case STONE: glColor3d(0.3, 0.3, 0.3); break;
                    case TREE: glColor3d(0.0, 0.6, 0.0); break;
                    case WATER: glColor3d(0.6, 0.8, 1.0); break;
                    case ARMORY_1:
                    case ARMORY_2:
                    case MEDICNE_1:
                    case MEDICNE_2:
                        glColor3d(1.0, 0.9, 0.2); 
                        break;
                    default: glColor3d(0.9, 0.9, 0.9); break;
                    }
                }
                else {
					//not visible by team black
                    glColor3d(0.1, 0.1, 0.1);
                }
                glBegin(GL_POLYGON);
                glVertex2d(i, j); glVertex2d(i, j + 1); glVertex2d(i + 1, j + 1); glVertex2d(i + 1, j);
                glEnd();
            }
        }
    }
}


