#include "Bullet.h"
#include "Definitions.h"
#include "Grenade.h"
#include "Map.h"
#include "NPC.h"
#include "SecurityMap.h"
#include "WarriorNPC.h"
#include "glut.h"
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <time.h>

using namespace std;

int gameWinner = 0; // 0 = playing, 1 = team1, 2 = team2

const int W = 900;
const int H = 600;

NPC *team1[TEAM_SIZE];
NPC *team2[TEAM_SIZE];

enum NPCType { Warrior_1 = 0, Warrior_2 = 1, Medic = 2, Supply = 3 };

void drawText(double x, double y, const char *text, void *font) {
  glRasterPos2d(x, y);
  while (*text) {
    glutBitmapCharacter(font, *text);
    text++;
  }
}

void init() {
  srand(1796);
  glClearColor(0, 0.5, 0.8, 0);
  glOrtho(0, 100, 0, 100, -1, 1);
  InitMap(team1, team2);
  CreateSecurityMap();

  cout << GAME_END << R"(
    _    ___   ____        _     _ _               
   / \  |_ _| / ___|  ___ | | __| (_) ___ _ __ ___ 
  / _ \  | |  \___ \ / _ \| |/ _` | |/ _ \ '__/ __|
 / ___ \ | |   ___) | (_) | | (_| | |  __/ |  \__ \
/_/   \_\___| |____/ \___/|_|\__,_|_|\___|_|  |___/
)" << RESET
       << endl;

  cout << TEAM1 << "Team 1: 2 Warriors + 1 Medic + 1 Supply" << RESET << endl;
  cout << TEAM2 << "Team 2: 2 Warriors + 1 Medic + 1 Supply" << RESET << endl;
  cout << "Map: " << numRooms << " rooms connected by passages" << endl;
}

void showBullet(NPCType warrior, NPC **team) {
  if (auto wn = dynamic_cast<WarriorNPC *>(team[warrior])) {
    if (wn->getBullet())
      wn->getBullet()->Show();
  }
}

void showGranade(NPCType warrior, NPC **team) {
  if (auto wn = dynamic_cast<WarriorNPC *>(team[warrior])) {
    if (wn->getGrenade())
      wn->getGrenade()->Show();
  }
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  DrawMap();

  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team1[i] != nullptr)
      team1[i]->show();
    if (team2[i] != nullptr)
      team2[i]->show();
  }

  showBullet(NPCType::Warrior_1, team1);
  showBullet(NPCType::Warrior_2, team1);
  showBullet(NPCType::Warrior_1, team2);
  showBullet(NPCType::Warrior_2, team2);

  showGranade(NPCType::Warrior_1, team1);
  showGranade(NPCType::Warrior_2, team1);
  showGranade(NPCType::Warrior_1, team2);
  showGranade(NPCType::Warrior_2, team2);

  if (gameWinner != 0) {
    glColor3d(0.0, 0.0, 0.0);
    void *font = GLUT_BITMAP_TIMES_ROMAN_24;
    string message = "";
    if (gameWinner == 1)
      message = "TEAM 1 WINS!";
    else if (gameWinner == 2)
      message = "TEAM 2 WINS!";
    else
      message = "DRAW!";
    drawText(40, 50, message.c_str(), font);
  }

  glutSwapBuffers();
}

void displaySecurityMap() {
  glClear(GL_COLOR_BUFFER_BIT);
  DrawSecurityMap();
  glutSwapBuffers();
}

void BulletMovement(NPCType warrior, NPC **team) {
  if (auto wn = dynamic_cast<WarriorNPC *>(team[warrior])) {
    if (wn->getBullet() != nullptr) {
      if (wn->getBullet()->getIsMoving()) {
        wn->getBullet()->Move(map, team1, team2, securityMap);
      } else {
        delete wn->getBullet();
        wn->setBullet(nullptr);
      }
    }
  }
}

void GranadeMovement(NPCType warrior, NPC **team) {
  if (auto wn = dynamic_cast<WarriorNPC *>(team[warrior])) {
    if (wn->getGrenade() != nullptr) {
      if (wn->getGrenade()->getIsExploded()) {
        wn->getGrenade()->Explode(map, team1, team2, securityMap);
      } else {
        delete wn->getGrenade();
        wn->setGrenade(nullptr);
      }
    } else {
      wn->setGrenade(nullptr);
    }
  }
}

void idle() {
  if (gameWinner != 0) {
    glutPostRedisplay();
    return;
  }

  // Check for deaths
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team1[i] != nullptr && team1[i]->getHp() <= 0) {
      delete team1[i];
      team1[i] = nullptr;
    }
    if (team2[i] != nullptr && team2[i]->getHp() <= 0) {
      delete team2[i];
      team2[i] = nullptr;
    }
  }

  // Win condition
  bool team1Alive = false;
  bool team2Alive = false;

  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team1[i] != nullptr) {
      team1Alive = true;
      break;
    }
  }
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team2[i] != nullptr) {
      team2Alive = true;
      break;
    }
  }

  if (team1Alive && !team2Alive) {
    gameWinner = 1;
    cout << GAME_END
         << "###############\n# GAME OVER   #\n# TEAM 1 WINS "
            "#\n###############\n"
         << RESET << endl;
  } else if (!team1Alive && team2Alive) {
    gameWinner = 2;
    cout << GAME_END
         << "###############\n# GAME OVER   #\n# TEAM 2 WINS "
            "#\n###############\n"
         << RESET << endl;
  } else if (!team1Alive && !team2Alive) {
    gameWinner = 3;
    cout << GAME_END
         << "###############\n# GAME OVER   #\n#   DRAW!     "
            "#\n###############\n"
         << RESET << endl;
  }

  if (gameWinner != 0)
    return;

  // Visibility update for all NPCs
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team1[i])
      team1[i]->UpdateVisibility(team1, team2);
    if (team2[i])
      team2[i]->UpdateVisibility(team2, team1);
  }

  // AI update
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team1[i])
      team1[i]->DoSomeWork();
    if (team2[i])
      team2[i]->DoSomeWork();
  }

  // Bullet movement
  BulletMovement(NPCType::Warrior_1, team1);
  BulletMovement(NPCType::Warrior_2, team1);
  BulletMovement(NPCType::Warrior_1, team2);
  BulletMovement(NPCType::Warrior_2, team2);

  // Grenade movement
  GranadeMovement(NPCType::Warrior_1, team1);
  GranadeMovement(NPCType::Warrior_2, team1);
  GranadeMovement(NPCType::Warrior_1, team2);
  GranadeMovement(NPCType::Warrior_2, team2);

  glutPostRedisplay();
}

void MouseClick(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
  }
}

void menu(int choice) {
  switch (choice) {
  case 1:
    glutDisplayFunc(displaySecurityMap);
    break;
  case 2:
    glutDisplayFunc(display);
    break;
  }
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowSize(W, H);
  glutInitWindowPosition(400, 100);
  glutCreateWindow("AI Soldiers - Maze Battle");

  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutMouseFunc(MouseClick);

  glutCreateMenu(menu);
  glutAddMenuEntry("Show Security Map", 1);
  glutAddMenuEntry("Show Regular Map", 2);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  init();
  glutMainLoop();

  return 0;
}
