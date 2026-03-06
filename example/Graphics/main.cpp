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
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>

using namespace std;

int gameWinner = 0; // 0 = playing, 1 = team1, 2 = team2
bool gameStarted  = false;

const int MATCH_DURATION_MS = 60000; // 1 minute
int gameStartTime = 0;

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

// 5-line ASCII-art logo for the start screen (GLUT_BITMAP_9_BY_15, ~1 unit/char)
static void drawStartScreen() {
  // Full-screen dark overlay
  glColor3d(0.02, 0.02, 0.08);
  glBegin(GL_QUADS);
  glVertex2d(0, 0); glVertex2d(100, 0);
  glVertex2d(100, 100); glVertex2d(0, 100);
  glEnd();

  // 5-line "SMART WARFARE" ASCII art logo in cyan
  static const char *logo[5] = {
    " ____  __  __    _    ____ _____   __        ___    ____  _____ _    ____  ___ ",
    "/ ___||  \\/  |  / \\  |  _ \\_   _|  \\ \\      / / \\  |  _ \\|  ___/ \\  |  _ \\| __|",
    "\\___ \\| |\\/| | / _ \\ | |_) || |     \\ \\ /\\ / / _ \\ |    /| |_ / _ \\ |    /|  _|",
    " ___) | |  | |/ ___ \\|  _ < | |      \\ V  V / ___ \\| |\\ \\|  _/ ___ \\| |\\ \\| |___",
    "|____/|_|  |_/_/   \\_\\_| \\_\\|_|       \\_/\\_/_/   \\_\\_| \\_\\_|/_/   \\_\\_| \\_\\____|"
  };
  void *smallFont = GLUT_BITMAP_9_BY_15;
  glColor3d(0.0, 0.9, 1.0);
  for (int i = 0; i < 5; i++)
    drawText(10.0, 83.0 - i * 4.0, logo[i], smallFont);

  // Call-to-action
  glColor3d(0.0, 0.9, 1.0);
  drawText(30.0, 48.0, "Press any key to start", GLUT_BITMAP_TIMES_ROMAN_24);

  // Right-click hint
  glColor3d(0.7, 0.7, 0.7);
  drawText(2.0, 2.0, "Right-click for view menu", smallFont);
}

void init() {
  srand((unsigned)time(NULL));
  // Dark blue-black background
  glClearColor(0.01f, 0.01f, 0.04f, 0.0f);
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
  cout << "Match duration: 60 seconds" << endl;

  // gameStartTime is set when the player presses a key to start
}

void keyboard(unsigned char /*key*/, int /*x*/, int /*y*/) {
  if (!gameStarted) {
    gameStarted  = true;
    gameStartTime = glutGet(GLUT_ELAPSED_TIME);
  }
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

  // Timer (only while game is running)
  if (gameWinner == 0 && gameStarted) {
    int elapsed    = glutGet(GLUT_ELAPSED_TIME) - gameStartTime;
    int remaining  = (MATCH_DURATION_MS - elapsed) / 1000;
    if (remaining < 0) remaining = 0;
    int mins = remaining / 60;
    int secs = remaining % 60;
    char timerBuf[16];
    sprintf(timerBuf, "%d:%02d", mins, secs);

    // White normally, urgent red when < 10 s
    if (remaining < 10)
      glColor3d(1.0, 0.2, 0.2);
    else
      glColor3d(1.0, 1.0, 1.0);
    drawText(46, 97, timerBuf, GLUT_BITMAP_TIMES_ROMAN_24);
  }

  // Win / draw screen
  if (gameWinner != 0) {
    void *font = GLUT_BITMAP_TIMES_ROMAN_24;
    if (gameWinner == 1) {
      glColor3d(1.0, 0.4, 0.0);          // team 1 orange-red
      drawText(38, 54, "TEAM 1 WINS!", font);
    } else if (gameWinner == 2) {
      glColor3d(0.0, 0.8, 1.0);          // team 2 cyan
      drawText(38, 54, "TEAM 2 WINS!", font);
    } else {
      glColor3d(1.0, 0.8, 0.0);          // draw: yellow
      drawText(44, 54, "DRAW!", font);
    }
    glColor3d(0.7, 0.7, 0.7);
    drawText(26, 46, "Right-click to switch views", GLUT_BITMAP_9_BY_15);
  }

  // Start-screen overlay drawn last so it sits on top of the map preview
  if (!gameStarted)
    drawStartScreen();

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
  // Freeze everything until the player starts the match
  if (!gameStarted) {
    glutPostRedisplay();
    return;
  }

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

  // Win condition 1: a squad loses all its warriors
  bool team1WarriorsAlive = (team1[0] != nullptr) || (team1[1] != nullptr);
  bool team2WarriorsAlive = (team2[0] != nullptr) || (team2[1] != nullptr);

  if (!team1WarriorsAlive && team2WarriorsAlive) {
    gameWinner = 2;
    cout << GAME_END
         << "###############\n# GAME OVER   #\n# TEAM 2 WINS "
            "#\n# (warriors)  #\n###############\n"
         << RESET << endl;
  } else if (team1WarriorsAlive && !team2WarriorsAlive) {
    gameWinner = 1;
    cout << GAME_END
         << "###############\n# GAME OVER   #\n# TEAM 1 WINS "
            "#\n# (warriors)  #\n###############\n"
         << RESET << endl;
  } else if (!team1WarriorsAlive && !team2WarriorsAlive) {
    gameWinner = 3;
    cout << GAME_END
         << "###############\n# GAME OVER   #\n#   DRAW!     "
            "#\n###############\n"
         << RESET << endl;
  }

  // Win condition 2: time limit (1 minute) -- team with more total HP wins
  if (gameWinner == 0) {
    int elapsed = glutGet(GLUT_ELAPSED_TIME) - gameStartTime;
    if (elapsed >= MATCH_DURATION_MS) {
      double team1HP = 0, team2HP = 0;
      for (int i = 0; i < TEAM_SIZE; i++) {
        if (team1[i]) team1HP += team1[i]->getHp();
        if (team2[i]) team2HP += team2[i]->getHp();
      }
      if (team1HP > team2HP) {
        gameWinner = 1;
        cout << GAME_END
             << "###############\n# TIME'S UP!  #\n# TEAM 1 WINS "
                "#\n# (more HP)   #\n###############\n"
             << RESET << endl;
      } else if (team2HP > team1HP) {
        gameWinner = 2;
        cout << GAME_END
             << "###############\n# TIME'S UP!  #\n# TEAM 2 WINS "
                "#\n# (more HP)   #\n###############\n"
             << RESET << endl;
      } else {
        gameWinner = 3;
        cout << GAME_END
             << "###############\n# TIME'S UP!  #\n#   DRAW!     "
                "#\n###############\n"
             << RESET << endl;
      }
      cout << "Team 1 total HP: " << (int)team1HP
           << " | Team 2 total HP: " << (int)team2HP << endl;
    }
  }

  if (gameWinner != 0)
    return;

  // Security map: decay + update from NPC positions in combat rooms
  UpdateSecurityMap(team1, team2);

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
  glutKeyboardFunc(keyboard);

  glutCreateMenu(menu);
  glutAddMenuEntry("Show Security Map", 1);
  glutAddMenuEntry("Show Regular Map", 2);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  init();
  glutMainLoop();

  return 0;
}
