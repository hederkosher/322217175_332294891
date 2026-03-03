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

Bullet::Bullet(double xPos, double yPos, double angle, int team, double damage, double radius, bool pierce)
{
    x = xPos;
    y = yPos;
    dirX = cos(angle);
    dirY = sin(angle);
    isMoving = false;
    this->team = team;
    damageAmount = damage;
    drawRadius = radius;
    pierceEnemies = pierce;
    trailIdx = 0;
    trailCount = 0;
    for (int i = 0; i < TRAIL_LEN; i++) { trailX[i] = xPos; trailY[i] = yPos; }
}

void Bullet::Move(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ])
{
    if (!isMoving) return;

    trailX[trailIdx] = x;
    trailY[trailIdx] = y;
    trailIdx = (trailIdx + 1) % TRAIL_LEN;
    if (trailCount < TRAIL_LEN) trailCount++;

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
            targetTeam[i]->setHp(targetTeam[i]->getHp() - damageAmount);
            if (!pierceEnemies) {
                isMoving = false;
                return;
            }
            // Piercing (grenade particles): damage but keep moving
        }
    }

    x = nextX;
    y = nextY;
    securityMap[gridX][gridY] += SECURITY;
}

void Bullet::Show()
{
    if (!isMoving) return;

    double r, g, b;
    if (team == 1) { r = 1.0; g = 0.5; b = 0.0; }
    else           { r = 0.0; g = 0.8; b = 1.0; }

    // Neon trail line (thick, fading segments)
    if (trailCount > 1) {
        for (int i = 0; i < trailCount - 1; i++) {
            int idx0 = (trailIdx - 1 - i + TRAIL_LEN) % TRAIL_LEN;
            int idx1 = (trailIdx - 2 - i + TRAIL_LEN) % TRAIL_LEN;
            double fade = 1.0 - (double)i / trailCount;
            glColor3d(r * fade, g * fade, b * fade);
            glLineWidth((float)(2.5 * fade + 0.5));
            glBegin(GL_LINES);
            glVertex2d(trailX[idx0], trailY[idx0]);
            glVertex2d(trailX[idx1], trailY[idx1]);
            glEnd();
        }
        glLineWidth(1.0f);
    }

    // Bright neon bullet head
    glColor3d(r, g, b);
    glBegin(GL_POLYGON);
    glVertex2d(x - drawRadius, y);
    glVertex2d(x, y + drawRadius);
    glVertex2d(x + drawRadius, y);
    glVertex2d(x, y - drawRadius);
    glEnd();
}

bool Bullet::getIsMoving() const {
    return isMoving;
}
