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

void UpdateSecurityMap(NPC** team1, NPC** team2)
{
    // Decay all values each frame so old danger fades
    for (int i = 0; i < MSZ; i++)
        for (int j = 0; j < MSZ; j++)
            if (securityMap[i][j] > 0.0)
                securityMap[i][j] *= 0.995;

    // Determine which rooms have NPCs from each team
    bool team1InRoom[MAX_ROOMS + 1] = {};
    bool team2InRoom[MAX_ROOMS + 1] = {};
    for (int i = 0; i < TEAM_SIZE; i++) {
        if (team1[i] && team1[i]->getHp() > 0) {
            int r = team1[i]->getCurrentRoom();
            if (r > 0 && r <= MAX_ROOMS) team1InRoom[r] = true;
        }
        if (team2[i] && team2[i]->getHp() > 0) {
            int r = team2[i]->getCurrentRoom();
            if (r > 0 && r <= MAX_ROOMS) team2InRoom[r] = true;
        }
    }

    // For each NPC in a room where both teams are present, add security
    // influence around its position — only within that room's bounds
    for (int t = 0; t < 2; t++) {
        NPC** team    = (t == 0) ? team1 : team2;
        bool* enemyIn = (t == 0) ? team2InRoom : team1InRoom;

        for (int i = 0; i < TEAM_SIZE; i++) {
            NPC* npc = team[i];
            if (!npc || npc->getHp() <= 0) continue;
            int r = npc->getCurrentRoom();
            if (r <= 0 || r > MAX_ROOMS || !enemyIn[r]) continue;

            Room* room = GetRoomById(r);
            if (!room) continue;

            double px, py;
            npc->getPosition(px, py);
            int cx = (int)(px + 1.5);
            int cy = (int)(py + 1.5);

            // Spread influence in a small radius, clamped to room bounds
            for (int dx = -4; dx <= 4; dx++) {
                for (int dy = -4; dy <= 4; dy++) {
                    int nx = cx + dx, ny = cy + dy;
                    if (nx < room->x1 || nx > room->x2 ||
                        ny < room->y1 || ny > room->y2) continue;
                    double dist = sqrt((double)(dx * dx + dy * dy));
                    securityMap[nx][ny] += SECURITY / (1.0 + dist);
                    if (securityMap[nx][ny] > 1.0) securityMap[nx][ny] = 1.0;
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
