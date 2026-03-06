#include "SecurityMap.h"
#include "Map.h"
#include "NPC.h"
#include "Grenade.h"
#include <stdlib.h>
#include <math.h>
#include "glut.h"
#include <iostream>

double securityMap[MSZ][MSZ] = { 0 };

void CreateSecurityMap()
{
    for (int i = 0; i < MSZ; i++)
        for (int j = 0; j < MSZ; j++)
            securityMap[i][j] = 0.0;
}

// Decay and add risk around enemies in fight rooms (so pathfinding biases away from danger)
void UpdateSecurityMapForCombatRooms(NPC** team1, NPC** team2)
{
    const double DECAY = 0.97;
    const double ENEMY_RISK_RADIUS = 8.0;
    const double RISK_AT_ENEMY = 0.15;

    for (int i = 0; i < MSZ; i++)
        for (int j = 0; j < MSZ; j++)
            securityMap[i][j] *= DECAY;

    for (int r = 1; r <= numRooms; r++) {
        bool team1InRoom = false, team2InRoom = false;
        for (int k = 0; k < TEAM_SIZE; k++) {
            if (team1[k] && team1[k]->getHp() > 0 && team1[k]->getCurrentRoom() == r) team1InRoom = true;
            if (team2[k] && team2[k]->getHp() > 0 && team2[k]->getCurrentRoom() == r) team2InRoom = true;
        }
        if (!team1InRoom || !team2InRoom) continue;

        for (int t = 0; t < 2; t++) {
            NPC** enemies = (t == 0) ? team2 : team1;
            for (int k = 0; k < TEAM_SIZE; k++) {
                if (!enemies[k] || enemies[k]->getHp() <= 0 || enemies[k]->getCurrentRoom() != r) continue;
                double ex, ey;
                enemies[k]->getPosition(ex, ey);
                int ei = (int)(ex + 1.5), ej = (int)(ey + 1.5);
                for (int di = - (int)ENEMY_RISK_RADIUS; di <= (int)ENEMY_RISK_RADIUS; di++) {
                    for (int dj = - (int)ENEMY_RISK_RADIUS; dj <= (int)ENEMY_RISK_RADIUS; dj++) {
                        int ni = ei + di, nj = ej + dj;
                        if (ni < 0 || ni >= MSZ || nj < 0 || nj >= MSZ || roomId[ni][nj] != r) continue;
                        double dist = sqrt((double)(di*di + dj*dj));
                        if (dist > ENEMY_RISK_RADIUS) continue;
                        double add = RISK_AT_ENEMY * (1.0 - dist / ENEMY_RISK_RADIUS);
                        securityMap[ni][nj] += add;
                    }
                }
            }
        }
    }
}

void DrawSecurityMap()
{
    for (int i = 0; i < MSZ; i++) {
        for (int j = 0; j < MSZ; j++) {
            switch (map[i][j]) {
            case FLOOR:
            {
                // Only show security values in rooms (where combat happens)
                double sec = securityMap[i][j];
                if (roomId[i][j] > 0 && sec > 0.0) {
                    double r = sec * 5.0;
                    if (r > 1.0) r = 1.0;
                    glColor3d(1.0, 1.0 - r, 1.0 - r);
                }
                else if (roomId[i][j] > 0) {
                    glColor3d(0.9, 0.9, 0.85);
                }
                else {
                    glColor3d(0.7, 0.7, 0.65);
                }
                break;
            }
            case WALL:
                glColor3d(0.2, 0.2, 0.25);
                break;
            case STONE:
                glColor3d(0.45, 0.42, 0.38);
                break;
            case ARMORY:
                glColor3d(0.9, 0.75, 0.1);
                break;
            case MEDICINE:
                glColor3d(0.9, 0.2, 0.3);
                break;
            default:
                glColor3d(0.2, 0.2, 0.25);
                break;
            }

            glBegin(GL_POLYGON);
            glVertex2d(i, j);
            glVertex2d(i, j + 1);
            glVertex2d(i + 1, j + 1);
            glVertex2d(i + 1, j);
            glEnd();
        }
    }
}
