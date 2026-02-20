#include "Bullet.h"
#include <math.h>
#include "glut.h"


namespace {
    bool CheckCollision(double bulletX, double bulletY, NPC* targetNpc) {
        if (!targetNpc) return false;

        double npcX, npcY;
        targetNpc->getPosition(npcX, npcY);

        int bulletGridX = static_cast<int>(bulletX);
        int bulletGridY = static_cast<int>(bulletY);
        int npcGridX = static_cast<int>(npcX);
        int npcGridY = static_cast<int>(npcY);

        bool hitX = (bulletGridX >= npcGridX && bulletGridX < npcGridX + 3);
        bool hitY = (bulletGridY >= npcGridY && bulletGridY < npcGridY + 3);

        return hitX && hitY;
    }
}

Bullet::Bullet(double xPos, double yPos, double angle, int team)
{
    x = xPos;
    y = yPos;
    dirX = cos(angle);
    dirY = sin(angle);
    isMoving = false;
    this->team = team;
}

void Bullet::Move(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ])
{
    if (!isMoving) return;

    const double DAMAGE_HP = 60.0;
    double nextX = x + SPEED * dirX;
    double nextY = y + SPEED * dirY;

    int gridX = static_cast<int>(nextX);
    int gridY = static_cast<int>(nextY);

    if (nextX < 0 || nextX >= MSZ || nextY < 0 || nextY >= MSZ) {
        isMoving = false;
        return;
    }

    int value = map[gridX][gridY];

    // Bullets stop at walls and stone obstacles
    if (value == WALL || value == STONE) {
        isMoving = false;
        return;
    }

    // Only hit enemies, no friendly fire
    NPC** targetTeam = (team == 1) ? team2 : team1;

    for (int i = 0; i < TEAM_SIZE; i++)
    {
        if (CheckCollision(nextX, nextY, targetTeam[i]))
        {
            targetTeam[i]->setHp(targetTeam[i]->getHp() - DAMAGE_HP);
            isMoving = false;
            return;
        }
    }

    x = nextX;
    y = nextY;
    securityMap[gridX][gridY] += SECURITY;
}

void Bullet::Show()
{
    if (!isMoving) return;
    glColor3d(1, 0, 0);
    glBegin(GL_POLYGON);
    glVertex2d(x - 0.5, y);
    glVertex2d(x, y + 0.5);
    glVertex2d(x + 0.5, y);
    glVertex2d(x, y - 0.5);
    glEnd();
}

bool Bullet::getIsMoving() const {
    return isMoving;
}
