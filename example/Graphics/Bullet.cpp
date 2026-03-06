#include "Bullet.h"
#include <algorithm>
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

    // Walk segment (x0,y0)->(x1,y1) and return true if any cell is WALL or STONE.
    // Use Bresenham so we visit every grid cell the segment crosses (no missed corners).
    bool SegmentHitsObstacle(int map[MSZ][MSZ], double x0, double y0, double x1, double y1) {
        int ix0 = static_cast<int>(x0), iy0 = static_cast<int>(y0);
        int ix1 = static_cast<int>(x1), iy1 = static_cast<int>(y1);

        if (ix0 == ix1 && iy0 == iy1) {
            if (ix0 < 0 || ix0 >= MSZ || iy0 < 0 || iy0 >= MSZ) return true;
            return (map[ix0][iy0] == WALL || map[ix0][iy0] == STONE);
        }

        int dx = std::abs(ix1 - ix0), sx = (ix0 < ix1) ? 1 : -1;
        int dy = -std::abs(iy1 - iy0), sy = (iy0 < iy1) ? 1 : -1;
        int err = dx + dy;

        for (;;) {
            if (ix0 < 0 || ix0 >= MSZ || iy0 < 0 || iy0 >= MSZ)
                return true;
            if (map[ix0][iy0] == WALL || map[ix0][iy0] == STONE)
                return true;
            if (ix0 == ix1 && iy0 == iy1)
                break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; ix0 += sx; }
            if (e2 <= dx) { err += dx; iy0 += sy; }
        }
        return false;
    }
}

Bullet::Bullet(double xPos, double yPos, double angle, int team, double damage)
{
    x = xPos;
    y = yPos;
    dirX = cos(angle);
    dirY = sin(angle);
    isMoving = false;
    this->team = team;
    this->damage = damage;
}

void Bullet::Move(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ])
{
    if (!isMoving) return;

    double nextX = x + SPEED * dirX;
    double nextY = y + SPEED * dirY;

    if (nextX < 0 || nextX >= MSZ || nextY < 0 || nextY >= MSZ) {
        isMoving = false;
        return;
    }

    // Check every cell along the segment; stop at first obstacle (no enemy hit)
    if (SegmentHitsObstacle(map, x, y, nextX, nextY)) {
        isMoving = false;
        return;
    }

    int gridX = static_cast<int>(nextX);
    int gridY = static_cast<int>(nextY);

    // Only hit enemies, no friendly fire
    NPC** targetTeam = (team == 1) ? team2 : team1;

    for (int i = 0; i < TEAM_SIZE; i++)
    {
        if (CheckCollision(nextX, nextY, targetTeam[i]))
        {
            targetTeam[i]->setHp(targetTeam[i]->getHp() - damage);
            isMoving = false;
            return;
        }
    }

    x = nextX;
    y = nextY;
    securityMap[gridX][gridY] += SECURITY;
}

void Bullet::Move(int map[MSZ][MSZ], NPC** team1, NPC** team2,
                  double securityMap[MSZ][MSZ],
                  bool hitTeam1[TEAM_SIZE], bool hitTeam2[TEAM_SIZE])
{
    if (!isMoving) return;

    double nextX = x + SPEED * dirX;
    double nextY = y + SPEED * dirY;

    if (nextX < 0 || nextX >= MSZ || nextY < 0 || nextY >= MSZ) {
        isMoving = false;
        return;
    }

    // Check every cell along the segment; stop at first obstacle (no enemy hit)
    if (SegmentHitsObstacle(map, x, y, nextX, nextY)) {
        isMoving = false;
        return;
    }

    int gridX = static_cast<int>(nextX);
    int gridY = static_cast<int>(nextY);

    // Only hit enemies, no friendly fire
    NPC** targetTeam = (team == 1) ? team2 : team1;

    for (int i = 0; i < TEAM_SIZE; i++)
    {
        if (CheckCollision(nextX, nextY, targetTeam[i]))
        {
            // Ensure each NPC is damaged at most once per grenade explosion
            bool* hitArray = (team == 1) ? hitTeam2 : hitTeam1;
            if (!hitArray[i]) {
                targetTeam[i]->setHp(targetTeam[i]->getHp() - damage);
                hitArray[i] = true;
            }
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
