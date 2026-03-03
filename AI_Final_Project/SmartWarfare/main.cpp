#include "Bullet.h"
#include "Definitions.h"
#include "Grenade.h"
#include "Map.h"
#include "MedicNPC.h"
#include "NPC.h"
#include "SecurityMap.h"
#include "SupplyNPC.h"
#include "WarriorNPC.h"
#include "glut.h"
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <cstdio>
#include <sstream>
#include <string>
#include <time.h>

using namespace std;

int gameWinner = 0; // 0 = playing, 1 = team1, 2 = team2, 3 = draw
const int MATCH_DURATION_MS = 60 * 1000;
int matchStartTime = 0;
bool timeExpired = false;
bool gameStarted = false;

const int TARGET_FPS = 160;
const int FRAME_MS = 1000 / TARGET_FPS;

// FPS counter (bottom-left)
static int debugFrameCount = 0;
static int debugLastFpsTime = 0;
static int debugFps = 0;

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

static void drawLogo(double x, double y, void *font) {
  static const char *logo[] = {
    "  ____                       _    __        __         __                ",
    " / ___| _ __ ___   __ _ _ __| |_  \\ \\      / /_ _ _ __/ _| __ _ _ __ ___ ",
    " \\___ \\| '_ ` _ \\ / _` | '__| __|  \\ \\ /\\ / / _` | '__| |_ / _` | '__/ _ \\",
    "  ___) | | | | | | (_| | |  | |_    \\ V  V / (_| | |  |  _| (_| | | |  __/",
    " |____/|_| |_| |_|\\__,_|_|   \\__|    \\_/\\_/ \\__,_|_|  |_|  \\__,_|_|  \\___|"
  };
  for (int i = 0; i < 5; i++) {
    glRasterPos2d(x, y - i * 2.2);
    const char *p = logo[i];
    while (*p) {
      glutBitmapCharacter(font, *p);
      p++;
    }
  }
}

void init() {
  srand((unsigned)time(nullptr));
  glClearColor(0.01f, 0.01f, 0.04f, 0.0f);
  glOrtho(0, 100, 0, 100, -1, 1);
  InitMap(team1, team2);
  CreateSecurityMap();

  cout << GAME_END << R"(
  ____                       _    __        __         __                
 / ___| _ __ ___   __ _ _ __| |_  \ \      / /_ _ _ __/ _| __ _ _ __ ___ 
 \___ \| '_ ` _ \ / _` | '__| __|  \ \ /\ / / _` | '__| |_ / _` | '__/ _ \
  ___) | | | | | | (_| | |  | |_    \ V  V / (_| | |  |  _| (_| | | |  __/
 |____/|_| |_| |_|\__,_|_|   \__|    \_/\_/ \__,_|_|  |_|  \__,_|_|  \___|
)" << RESET
       << endl;

  cout << TEAM1 << "Team 1: 2 Warriors + 1 Medic + 1 Supply" << RESET << endl;
  cout << TEAM2 << "Team 2: 2 Warriors + 1 Medic + 1 Supply" << RESET << endl;
  cout << "Map: " << numRooms << " rooms, " << numArmories << " armories, " << numMedicine << " medicine depots" << endl;
  cout << "Match duration: 1:00" << endl;
}

static void PrintWarriorStats() {
  const char* labels[] = { "W1", "W2", "M", "P" };
  cout << "\n" << TEAM1 << "--- TEAM 1 ---" << RESET << endl;
  for (int i = 0; i < TEAM_SIZE; i++) {
    NPC* n = team1[i];
    if (!n || n->getHp() <= 0) continue;
    cout << "  " << labels[i] << ": " << (int)n->getHp() << " HP";
    if (auto w = dynamic_cast<WarriorNPC*>(n))
      cout << ", " << (int)w->getAmmo() << " Ammo";
    else if (auto m = dynamic_cast<MedicNPC*>(n))
      cout << ", " << (int)m->getMedicine() << " Med";
    else if (auto s = dynamic_cast<SupplyNPC*>(n))
      cout << ", " << (int)s->getAmmo() << " Pack";
    cout << " | agg=" << (int)(n->getAggressiveness() * 100) << "% cau=" << (int)(n->getCautiousness() * 100) << "%";
    cout << endl;
  }
  cout << TEAM2 << "--- TEAM 2 ---" << RESET << endl;
  for (int i = 0; i < TEAM_SIZE; i++) {
    NPC* n = team2[i];
    if (!n || n->getHp() <= 0) continue;
    cout << "  " << labels[i] << ": " << (int)n->getHp() << " HP";
    if (auto w = dynamic_cast<WarriorNPC*>(n))
      cout << ", " << (int)w->getAmmo() << " Ammo";
    else if (auto m = dynamic_cast<MedicNPC*>(n))
      cout << ", " << (int)m->getMedicine() << " Med";
    else if (auto s = dynamic_cast<SupplyNPC*>(n))
      cout << ", " << (int)s->getAmmo() << " Pack";
    cout << " | agg=" << (int)(n->getAggressiveness() * 100) << "% cau=" << (int)(n->getCautiousness() * 100) << "%";
    cout << endl;
  }
  cout << endl;
}

static void PrintFinalGameStats() {
  const char* labels[] = { "W1", "W2", "M", "P" };
  cout << "\n" << GAME_END << "=============== GAME OVER ===============" << RESET << endl;
  if (gameWinner == 1)
    cout << GAME_END << "     *** TEAM 1 WINS! ***" << RESET << endl;
  else if (gameWinner == 2)
    cout << GAME_END << "     *** TEAM 2 WINS! ***" << RESET << endl;
  else
    cout << GAME_END << "     *** DRAW! ***" << RESET << endl;
  cout << endl;
  cout << TEAM1 << "--- TEAM 1 STATS ---" << RESET << endl;
  for (int i = 0; i < TEAM_SIZE; i++) {
    NPC* n = team1[i];
    if (!n || n->getHp() <= 0) {
      cout << "  " << labels[i] << ": DEAD" << endl;
      continue;
    }
    cout << "  " << labels[i] << ": " << (int)n->getHp() << " HP";
    if (auto w = dynamic_cast<WarriorNPC*>(n))
      cout << ", " << (int)w->getAmmo() << " Ammo";
    else if (auto m = dynamic_cast<MedicNPC*>(n))
      cout << ", " << (int)m->getMedicine() << " Med";
    else if (auto s = dynamic_cast<SupplyNPC*>(n))
      cout << ", " << (int)s->getAmmo() << " Pack";
    cout << " | agg=" << (int)(n->getAggressiveness() * 100) << "% cau=" << (int)(n->getCautiousness() * 100) << "%";
    cout << endl;
  }
  cout << TEAM2 << "--- TEAM 2 STATS ---" << RESET << endl;
  for (int i = 0; i < TEAM_SIZE; i++) {
    NPC* n = team2[i];
    if (!n || n->getHp() <= 0) {
      cout << "  " << labels[i] << ": DEAD" << endl;
      continue;
    }
    cout << "  " << labels[i] << ": " << (int)n->getHp() << " HP";
    if (auto w = dynamic_cast<WarriorNPC*>(n))
      cout << ", " << (int)w->getAmmo() << " Ammo";
    else if (auto m = dynamic_cast<MedicNPC*>(n))
      cout << ", " << (int)m->getMedicine() << " Med";
    else if (auto s = dynamic_cast<SupplyNPC*>(n))
      cout << ", " << (int)s->getAmmo() << " Pack";
    cout << " | agg=" << (int)(n->getAggressiveness() * 100) << "% cau=" << (int)(n->getCautiousness() * 100) << "%";
    cout << endl;
  }
  cout << GAME_END << "\nPress R to restart\n" << RESET << endl;
}

static void RestartGame() {
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team1[i]) { delete team1[i]; team1[i] = nullptr; }
    if (team2[i]) { delete team2[i]; team2[i] = nullptr; }
  }
  ReleaseAllDepots();
  gameWinner = 0;
  timeExpired = false;
  gameStarted = false;
  matchStartTime = 0;
  InitMap(team1, team2);
  CreateSecurityMap();
  glutPostRedisplay();
}

static void OnKey(unsigned char key, int x, int y) {
  (void)x;
  (void)y;
  if (gameWinner != 0 && (key == 'r' || key == 'R')) {
    RestartGame();
    return;
  }
  if (!gameStarted) {
    gameStarted = true;
    matchStartTime = glutGet(GLUT_ELAPSED_TIME);
    PrintWarriorStats();
  }
}

static void OnSpecialKey(int key, int x, int y) {
  (void)key;
  (void)x;
  (void)y;
  if (!gameStarted) {
    gameStarted = true;
    matchStartTime = glutGet(GLUT_ELAPSED_TIME);
    PrintWarriorStats();
  }
}

void showBullet(NPCType warrior, NPC **team) {
  if (auto wn = dynamic_cast<WarriorNPC *>(team[warrior])) {
    Bullet **bArr = wn->getBullets();
    for (int i = 0; i < wn->getMaxBullets(); i++)
      if (bArr[i]) bArr[i]->Show();
  }
}

void showGranade(NPCType warrior, NPC **team) {
  if (auto wn = dynamic_cast<WarriorNPC *>(team[warrior])) {
    if (wn->getGrenade())
      wn->getGrenade()->Show();
  }
}

static void DrawTeamStats(NPC **team, double x, int teamNum) {
  void *font = GLUT_BITMAP_9_BY_15;
  double y = 98.0;
  if (teamNum == 1)
    glColor3d(1.0, 0.45, 0.0);
  else
    glColor3d(0.0, 0.8, 1.0);
  glRasterPos2d(x, y);
  std::string title = "Team " + std::to_string(teamNum);
  for (char c : title) { glutBitmapCharacter(font, c); }
  y -= 1.8;

  const char *labels[] = { "W1", "W2", "M", "P" };
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (!team[i] || team[i]->getHp() <= 0) {
      glColor3d(0.5, 0.1, 0.1);
      glRasterPos2d(x, y);
      std::string line = std::string(labels[i]) + ": DEAD";
      for (char c : line) { glutBitmapCharacter(font, c); }
      y -= 1.8;
      continue;
    }
    glColor3d(0.7, 0.8, 0.9);
    std::ostringstream line;
    line << labels[i] << ": " << (int)team[i]->getHp() << " HP";
    if (auto w = dynamic_cast<WarriorNPC *>(team[i]))
      line << "  " << (int)w->getAmmo() << " Ammo";
    else if (auto m = dynamic_cast<MedicNPC *>(team[i]))
      line << "  " << (int)m->getMedicine() << " Med";
    else if (auto s = dynamic_cast<SupplyNPC *>(team[i]))
      line << "  " << (int)s->getAmmo() << " Pack";
    std::string str = line.str();
    glRasterPos2d(x, y);
    for (char c : str) { glutBitmapCharacter(font, c); }
    y -= 1.8;
  }
}

static int GetRemainingMs() {
  int elapsed = glutGet(GLUT_ELAPSED_TIME) - matchStartTime;
  int remaining = MATCH_DURATION_MS - elapsed;
  return remaining > 0 ? remaining : 0;
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

  // Team stats in top corners
  DrawTeamStats(team1, 2.0, 1);
  DrawTeamStats(team2, 62.0, 2);

  // Timer at top center
  {
    int remainMs = (!gameStarted) ? MATCH_DURATION_MS : ((gameWinner != 0) ? 0 : GetRemainingMs());
    int totalSec = remainMs / 1000;
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    char buf[16];
    sprintf_s(buf, sizeof(buf), "%02d:%02d", mins, secs);

    void *timerFont = GLUT_BITMAP_TIMES_ROMAN_24;
    if (totalSec <= 10 && gameWinner == 0)
      glColor3d(1.0, 0.2, 0.2);
    else
      glColor3d(1.0, 1.0, 1.0);
    drawText(46, 95, buf, timerFont);
  }

  // Debug: FPS counter (bottom-left)
  {
    int now = glutGet(GLUT_ELAPSED_TIME);
    debugFrameCount++;
    if (now - debugLastFpsTime >= 1000) {
      debugFps = debugFrameCount;
      debugFrameCount = 0;
      debugLastFpsTime = now;
    }
    char fpsBuf[24];
    sprintf_s(fpsBuf, sizeof(fpsBuf), "FPS: %d", debugFps);
    glColor3d(0.0, 1.0, 0.5);
    drawText(2, 3, fpsBuf, GLUT_BITMAP_9_BY_15);
  }

  if (gameWinner != 0) {
    void *font = GLUT_BITMAP_TIMES_ROMAN_24;
    if (timeExpired) {
      glColor3d(1.0, 0.8, 0.0);
      drawText(38, 53, "TIME'S UP!", font);
    }
    glColor3d(1.0, 1.0, 1.0);
    string message;
    if (gameWinner == 1)
      message = "TEAM 1 WINS!";
    else if (gameWinner == 2)
      message = "TEAM 2 WINS!";
    else
      message = "DRAW!";
    drawText(40, 50, message.c_str(), font);
    glColor3d(0.7, 0.7, 0.7);
    drawText(36, 45, "Press R to restart", font);
  }

  if (!gameStarted) {
    glColor3d(0.02, 0.02, 0.08);
    glBegin(GL_QUADS);
    glVertex2d(0, 0);
    glVertex2d(100, 0);
    glVertex2d(100, 100);
    glVertex2d(0, 100);
    glEnd();
    glColor3d(0.0, 0.9, 1.0);
    void *font = GLUT_BITMAP_TIMES_ROMAN_24;
    void *fontSmall = GLUT_BITMAP_9_BY_15;
    drawLogo(15, 78, fontSmall);   // Centered: logo ~70 units wide, (100-70)/2 = 15
    drawText(36, 48, "Press any key to start", font);  // Centered
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
    Bullet **bArr = wn->getBullets();
    for (int i = 0; i < wn->getMaxBullets(); i++) {
      if (bArr[i] != nullptr) {
        if (bArr[i]->getIsMoving()) {
          bArr[i]->Move(map, team1, team2, securityMap);
        } else {
          delete bArr[i];
          bArr[i] = nullptr;
        }
      }
    }
  }
}

void GranadeMovement(NPCType warrior, NPC **team) {
  if (auto wn = dynamic_cast<WarriorNPC *>(team[warrior])) {
    Grenade *g = wn->getGrenade();
    if (g == nullptr) return;

    if (g->getIsFlying() || g->getFuseTimer() > 0) {
      g->Update();
    } else if (g->getIsExploded()) {
      g->Explode(map, team1, team2, securityMap);
    } else {
      delete g;
      wn->setGrenade(nullptr);
    }
  }
}

static void OnTimer(int value) {
  (void)value;

  if (!gameStarted || gameWinner != 0) {
    glutPostRedisplay();
    glutTimerFunc(FRAME_MS, OnTimer, 0);
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

  // Timer expiry - decide winner by total HP
  if (GetRemainingMs() <= 0) {
    double hp1 = 0, hp2 = 0;
    for (int i = 0; i < TEAM_SIZE; i++) {
      if (team1[i]) hp1 += team1[i]->getHp();
      if (team2[i]) hp2 += team2[i]->getHp();
    }
    timeExpired = true;
    if (hp1 > hp2) {
      gameWinner = 1;
    } else if (hp2 > hp1) {
      gameWinner = 2;
    } else {
      gameWinner = 3;
    }
    PrintFinalGameStats();
    glutPostRedisplay();
    glutTimerFunc(FRAME_MS, OnTimer, 0);
    return;
  }

  // Win condition - team wiped
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
    PrintFinalGameStats();
  } else if (!team1Alive && team2Alive) {
    gameWinner = 2;
    PrintFinalGameStats();
  } else if (!team1Alive && !team2Alive) {
    gameWinner = 3;
    PrintFinalGameStats();
  }

  if (gameWinner != 0) {
    glutPostRedisplay();
    glutTimerFunc(FRAME_MS, OnTimer, 0);
    return;
  }

  g_pathFindBudget = 1;  // 1 path per frame caps cost (~25-30ms max); support runs first for spawn

  // Visibility update for all NPCs
  for (int i = 0; i < TEAM_SIZE; i++) {
    if (team1[i])
      team1[i]->UpdateVisibility(team1, team2);
    if (team2[i])
      team2[i]->UpdateVisibility(team2, team1);
  }

  // AI update: run support (medic 2, supply 3) before warriors (0, 1) so they get path budget at spawn
  for (int i = 2; i < TEAM_SIZE; i++) {
    if (team1[i]) team1[i]->DoSomeWork();
    if (team2[i]) team2[i]->DoSomeWork();
  }
  for (int i = 0; i < 2; i++) {
    if (team1[i]) team1[i]->DoSomeWork();
    if (team2[i]) team2[i]->DoSomeWork();
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
  glutTimerFunc(FRAME_MS, OnTimer, 0);
}

void MouseClick(int button, int state, int x, int y) {
  (void)x;
  (void)y;
  if (!gameStarted && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    gameStarted = true;
    matchStartTime = glutGet(GLUT_ELAPSED_TIME);
    PrintWarriorStats();
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
  glutCreateWindow("Smart Warfare");

#if defined(_WIN32)
  // Disable vsync so FPS can exceed monitor refresh (e.g. 100+ on 75Hz display)
  typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
  PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
    (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
  if (wglSwapIntervalEXT)
    wglSwapIntervalEXT(0);
#endif

  glutDisplayFunc(display);
  glutTimerFunc(FRAME_MS, OnTimer, 0);
  glutKeyboardFunc(OnKey);
  glutSpecialFunc(OnSpecialKey);
  glutMouseFunc(MouseClick);

  glutCreateMenu(menu);
  glutAddMenuEntry("Show Security Map", 1);
  glutAddMenuEntry("Show Regular Map", 2);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  init();
  glutMainLoop();

  return 0;
}
