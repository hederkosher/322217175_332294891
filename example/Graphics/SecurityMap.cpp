#include "SecurityMap.h"
#include "Map.h"
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
