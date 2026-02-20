#include "SecurityMap.h"
#include "Map.h"        // for extern int map[MSZ][MSZ]
#include "Grenade.h"    // for Grenade and UpdateSecurityMap
#include <stdlib.h>
#include <math.h>
#include "glut.h"
#include <iostream>

class Grenade;       
extern Grenade* pg;     // defined in main.cpp

double securityMap[MSZ][MSZ] = { 0 }; // define the global security map

void CreateSecurityMap()
{

}

void DrawSecurityMap()
{
    int i, j;

    for (i = 0; i < MSZ; i++)
        for (j = 0; j < MSZ; j++)
        {
            switch (map[i][j])
            {
            case 0:
                glColor3d(1 - securityMap[i][j], 1 - securityMap[i][j], 1 - securityMap[i][j]);
                glBegin(GL_POLYGON);
                glVertex2d(i, j);
                glVertex2d(i, j + 1);
                glVertex2d(i + 1, j + 1);
                glVertex2d(i + 1, j);
                glEnd();
                break;
            case STONE:
                glColor3d(0.3, 0.3, 0.3);
                glBegin(GL_POLYGON);
                glVertex2d(i, j);
                glVertex2d(i, j + 1);
                glVertex2d(i + 1, j + 1);
                glVertex2d(i + 1, j);
                glEnd();
                break;
            case TREE:
                double baseLeftX = i;
                double baseRightX = i + TREE_W;
                double baseY = j;
                double apexX = i + TREE_W * 0.5;
                double apexY = j + TREE_H;
                glColor3d(0.3, 0.3, 0.3);
                // fill
                glColor3d(0.0, 0.6, 0.0);
                glBegin(GL_TRIANGLES);
                glVertex2d(apexX, apexY);
                glVertex2d(baseLeftX, baseY);
                glVertex2d(baseRightX, baseY);
                glEnd();

                glColor3d(0.0, 0.3, 0.0);
                glBegin(GL_LINE_LOOP);
                glVertex2d(apexX, apexY);
                glVertex2d(baseLeftX, baseY);
                glVertex2d(baseRightX, baseY);
                glEnd();
                break;
            }
           
        }

   

}
