#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "glut.h"
#include "Definitions.h"
#include "NPC.h"
#include "Bullet.h"
#include "Grenade.h"
#include "Map.h"
#include "SecurityMap.h"
#include <iostream>
#include <string> 
using namespace std;

// --- Global Game State ---
int gameWinner = 0; // 0 = playing, 1 = team 1 wins, 2 = team 2 wins

const int W = 900;
const int H = 600; // window sizes
const double PI = 3.14;

NPC* team1[TEAM_SIZE];
NPC* team2[TEAM_SIZE];

enum NPCType {
    Commander = 0,
    Warrior_1 = 1,
    Warrior_2 = 2,
    Medic = 3,
    Logistic = 4
};

// draw text on screen
void drawText(double x, double y, const char* text, void* font)
{
    glRasterPos2d(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}


void init()
{
    srand(1796); // for random
    glClearColor(0, 0.5, 0.8, 0);   // window background color
    glOrtho(0, 100, 0, 100, -1, 1);   // coordinate system
    InitMap(team1, team2);
    CreateSecurityMap();

    cout << GAME_END << R"(
    _    ___   ____        _     _ _               
   / \  |_ _| / ___|  ___ | | __| (_) ___ _ __ ___ 
  / _ \  | |  \___ \ / _ \| |/ _` | |/ _ \ '__/ __|
 / ___ \ | |   ___) | (_) | | (_| | |  __/ |  \__ \
/_/   \_\___| |____/ \___/|_|\__,_|_|\___|_|  |___/
)" << RESET << endl;
}

void showBullet(NPCType warrior, NPC** team)
{
    if (auto wn = dynamic_cast<WarriorNPC*>(team[warrior])) {
        if (wn->getBullet())
        {
            wn->getBullet()->Show();
        }
    }
}
void showGranade(NPCType warrior, NPC** team)
{
    if (auto wn = dynamic_cast<WarriorNPC*>(team[warrior])) {
        if (wn->getGrenade())
        {
            wn->getGrenade()->Show();
        }
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    DrawMap();

    // Draw Team 1
    for (int i = 0; i < TEAM_SIZE; i++)
    {
        if (team1[i] == nullptr) continue; // Don't draw if dead
        team1[i]->show();
    }
    // Draw Team 2
    for (int i = 0; i < TEAM_SIZE; i++)
    {
        if (team2[i] == nullptr) continue; // Don't draw if dead
        team2[i]->show();
    }

     //Display bullets
    showBullet(NPCType::Warrior_1, team1);
    showBullet(NPCType::Warrior_2, team1);
    showBullet(NPCType::Warrior_1, team2);
    showBullet(NPCType::Warrior_2, team2);

    //Display grenades
    showGranade(NPCType::Warrior_1, team1);
    showGranade(NPCType::Warrior_2, team1);
    showGranade(NPCType::Warrior_1, team2);
    showGranade(NPCType::Warrior_2, team2);

    // Draw Game Over Text if game has ended
    if (gameWinner != 0)
    {
        // Set text color (black)
        glColor3d(0.0, 0.0, 0.0);

        // Set font
        void* font = GLUT_BITMAP_TIMES_ROMAN_24;

        // Prepare the message
        string message = "";
        if (gameWinner == 1) {
            message = "TEAM 1 WINS!";
        }
        else {
            message = "TEAM 2 WINS!";
        }

        // Draw the text (coordinates are 0-100)
        drawText(40, 50, message.c_str(), font);
    }

    glutSwapBuffers();
}

void displaySecurityMap()
{
    glClear(GL_COLOR_BUFFER_BIT);

    DrawSecurityMap();

    glutSwapBuffers();
}

void displayVisibilityMapTeam1()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (auto co = dynamic_cast<CommanderNPC*>(team1[NPCType::Commander])) {
        co->DrawVisibilityMap();
    }

    glutSwapBuffers();
}

void displayVisibilityMapTeam2()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (auto co = dynamic_cast<CommanderNPC*>(team2[NPCType::Commander])) {
        co->DrawVisibilityMap();
    }

    glutSwapBuffers();
}

void BulletMovement(NPCType warrior, NPC** team)
{
    if (auto wn = dynamic_cast<WarriorNPC*>(team[warrior])) {
        if (wn->getBullet() != nullptr)
        {
            if (wn->getBullet()->getIsMoving()) {
                wn->getBullet()->Move(map, team1, team2, securityMap);
            }
            else {
                delete wn->getBullet();
                wn->setBullet(nullptr);
            }
        }
    }
}

void GranadeMovement(NPCType warrior, NPC** team)
{
    if (auto wn = dynamic_cast<WarriorNPC*>(team[warrior])) {
        if (wn->getGrenade() != nullptr)
        {
            if (wn->getGrenade()->getIsExploded()) {
                wn->getGrenade()->Explode(map, team1, team2, securityMap);
            }
            else {
                delete wn->getGrenade();
                wn->setGrenade(nullptr);
            }
        }
        else
        {
            wn->setGrenade(nullptr);
        }
    }
}

void idle()
{
    if (gameWinner != 0)
    {
        glutPostRedisplay(); // Keep redrawing the victory screen
        return; // Stop processing game logic
    }

    //Check for deaths
    for (int i = 0; i < TEAM_SIZE; i++)
    {
        if (auto c1 = dynamic_cast<CommanderNPC*>(team1[0])) {
            if (c1->getHp() <= 0)//if commander is dead, let the team search and stay solo
            {
				c1->orderTeamToSearch();
            }
        }


        if (team1[i] != nullptr && team1[i]->getHp() <= 0)
        {
            delete team1[i];
            team1[i] = nullptr;
        }
        if (team2[i] != nullptr && team2[i]->getHp() <= 0)
        {
            delete team2[i];
            team2[i] = nullptr;
        }
    }

    // check for Winning Condition
    bool team1HasLivingMembers = false;
    bool team2HasLivingMembers = false;

    // Check if Team 1 has anyone alive
    for (int i = 0; i < TEAM_SIZE; i++) {
        if (team1[i] != nullptr) {
            team1HasLivingMembers = true;
            break; // Found one, no need to check further
        }
    }

    // Check if Team 2 has anyone alive
    for (int i = 0; i < TEAM_SIZE; i++) {
        if (team2[i] != nullptr) {
            team2HasLivingMembers = true;
            break; // Found one, no need to check further
        }
    }

    // determine the winner
    if (team1HasLivingMembers && !team2HasLivingMembers) {
        gameWinner = 1; // Team 1 wins!
        cout << GAME_END <<
            "###############\n"
            "# GAME OVER   #\n"
            "# TEAM 1 WINS #\n"
            "###############\n"
            << RESET << endl;
    }
    else if (!team1HasLivingMembers && team2HasLivingMembers) {
        gameWinner = 2; // Team 2 wins!
        cout << GAME_END <<
            "###############\n"
            "# GAME OVER   #\n"
            "# TEAM 2 WINS #\n"
            "###############\n"
            << RESET << endl;
    }
    else if (!team1HasLivingMembers && !team2HasLivingMembers) {
        gameWinner = 3; // Or 0, if you want it to just stop
        cout << GAME_END <<
            "###############\n"
            "# GAME OVER   #\n"
            "#   DRAW!     #\n"
            "###############\n"
            << RESET << endl;
    }


    //  the game just ended, stop
    if (gameWinner != 0) return;

    //Visibility update
    if (auto co1 = dynamic_cast<CommanderNPC*>(team1[NPCType::Commander])) {
        co1->UpdateVisibility(team1, team2);
    }

    if (auto co2 = dynamic_cast<CommanderNPC*>(team2[NPCType::Commander])) {
        co2->UpdateVisibility(team2, team1);
    }

    // Team 1 AI
    if (team1[NPCType::Commander] != nullptr)
        team1[NPCType::Commander]->DoSomeWork();
    if (team1[NPCType::Warrior_1] != nullptr)
        team1[NPCType::Warrior_1]->DoSomeWork();
    if (team1[NPCType::Warrior_2] != nullptr)
        team1[NPCType::Warrior_2]->DoSomeWork();
    if (team1[NPCType::Medic] != nullptr)
        team1[NPCType::Medic]->DoSomeWork();
    if (team1[NPCType::Logistic] != nullptr)
        team1[NPCType::Logistic]->DoSomeWork();

    // Team 2 AI
    if (team2[NPCType::Commander] != nullptr)
        team2[NPCType::Commander]->DoSomeWork();
    if (team2[NPCType::Warrior_1] != nullptr)
        team2[NPCType::Warrior_1]->DoSomeWork();
    if (team2[NPCType::Warrior_2] != nullptr)
        team2[NPCType::Warrior_2]->DoSomeWork();
    if (team2[NPCType::Medic] != nullptr)
        team2[NPCType::Medic]->DoSomeWork();
    if (team2[NPCType::Logistic] != nullptr)
        team2[NPCType::Logistic]->DoSomeWork();

    // Bullet movement
    BulletMovement(NPCType::Warrior_1, team1);
    BulletMovement(NPCType::Warrior_2, team1);
    BulletMovement(NPCType::Warrior_1, team2);
    BulletMovement(NPCType::Warrior_2, team2);

    //Grenade movement
    GranadeMovement(NPCType::Warrior_1, team1);
    GranadeMovement(NPCType::Warrior_2, team1);
    GranadeMovement(NPCType::Warrior_1, team2);
    GranadeMovement(NPCType::Warrior_2, team2);

    glutPostRedisplay();
}

void MouseClick(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
    }
}

void menu(int choice)
{
    switch (choice)
    {
    case 1:
        glutDisplayFunc(displayVisibilityMapTeam1);
        break;
    case 2:
        glutDisplayFunc(displayVisibilityMapTeam2);
        break;
    case 3:
        glutDisplayFunc(displaySecurityMap);
        break;
    case 4:
        glutDisplayFunc(display);
        break;
    }
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(W, H);
    glutInitWindowPosition(400, 100);
    glutCreateWindow("AI Soldiers Games");

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMouseFunc(MouseClick);

    // menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Show Commander 1 Visibility Map", 1);
    glutAddMenuEntry("Show Commander 2 Visibility Map", 2);
    glutAddMenuEntry("Show Security Map", 3);
    glutAddMenuEntry("Show Regular Map", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    init();
    glutMainLoop();

    return 0;
}