#include "Map.h"
#include <stdlib.h>
#include <math.h>
#include "glut.h"
#include <iostream>
using namespace std;

// ---- Globals ----
int map[MSZ][MSZ] = { 0 };


const int TREE_W = 3;
const int TREE_H = 4;
const int TREE_COUNT = 10;

int treeX[TREE_COUNT] = { 0 };
int treeY[TREE_COUNT] = { 0 };

static void placeStones()
{
    const int num_stones = 9;
    const int stone_size = 3;

    for (int s = 0; s < num_stones; s++)
    {
        int x = rand() % (MSZ - stone_size);
        int y = rand() % (MSZ - stone_size);

        for (int i = 0; i < stone_size; i++)
            for (int j = 0; j < stone_size; j++)
                map[x + i][y + j] = STONE;
    }
}

static void placeWater()
{
    const int num_water = 2;
    const int water_size = 3;

    for (int s = 0; s < num_water; s++)
    {
        // Limit placement to middle third of the map
        int x_min = MSZ / 3;
        int x_max = 2 * MSZ / 3 - water_size;
        int y_min = water_size;
        int y_max = MSZ - water_size;

        int x = x_min + rand() % (x_max - x_min);
        int y = y_min + rand() % (y_max - y_min);

        // Create 3 blocks of 3x3 water squares (a column of water)
        for (int k = 0; k < 3; k++)
        {
            for (int i = 0; i < water_size; i++)
            {
                for (int j = 0; j < water_size; j++)
                {
                    map[x+ i + (k * water_size)][y  + j] = WATER;
                }
            }
        }
    }
}

void generateSquare(int type, NPC** npc, char symbol, int team, int x_min, int x_max, int y_min, int y_max, NPC** teamNPC)
{
    double x = x_min + rand() % (x_max - x_min);
    double y = y_min + rand() % (y_max - y_min);
    switch (type)
    {
    case LOGISTIC_1:
    case LOGISTIC_2:
        *npc = new LogisticNPC(x, y, symbol, team , type);
        break;
    case MEDIC_1:
    case MEDIC_2:
        *npc = new MedicNPC(x, y, symbol, team , type);
        break;
    case WARRIOR_1_1:
    case WARRIOR_1_2:
    case WARRIOR_2_1:
    case WARRIOR_2_2:
        *npc = new WarriorNPC(x, y, symbol, team , type);
        break;
    case COMMANDER_1:
    case COMMANDER_2:
        *npc = new CommanderNPC(x, y, symbol, team , type, teamNPC);
        break;
    default:
        break;
    }

}

void generateSquareMapObject(int type, int x, int y)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            map[x + i][y + j] = type;
        }
    }

}


static void placeArmoryMedicne()
{
    int x_min, x_max, y_min, y_max;
    x_min = 3 * MSZ / 5;
    x_max = MSZ - 1;
    y_min = MSZ / 3;
    y_max = 2 * MSZ / 3;

    generateSquareMapObject(ARMORY_1, ARMORY_1_X, ARMORY_1_Y);
    generateSquareMapObject(MEDICNE_1, MEDICINE_1_X, MEDICINE_1_Y);

    x_min = 0;
    x_max = MSZ / 5;
    y_min = MSZ / 3;
    y_max = 2 * MSZ / 3;

    generateSquareMapObject(ARMORY_2, ARMORY_2_X, ARMORY_2_Y);
    generateSquareMapObject(MEDICNE_2, MEDICINE_2_X, MEDICINE_2_Y);
}


static void PlaceTroops(NPC** team1, NPC** team2)
{
    int x_min, x_max, y_min, y_max;

    // Right 2/5 of the map (from 60% to 100%)
    x_min = 3 * MSZ / 5;
    x_max = MSZ - 1;

    // Middle vertically
    y_min = MSZ / 3;
    y_max = 2 * MSZ / 3;

    generateSquare(COMMANDER_1, &team1[0], 'C', 1, x_min, x_max, y_min, y_max, team1);
    generateSquare(WARRIOR_1_1, &team1[1], 'W', 1, x_min, x_max, y_min, y_max , nullptr);
    generateSquare(WARRIOR_1_2, &team1[2], 'W', 1, x_min, x_max, y_min, y_max , nullptr);
    generateSquare(MEDIC_1, &team1[3], 'M', 1, x_min , x_max, y_min, y_max , nullptr);
    generateSquare(LOGISTIC_1, &team1[4], 'P', 1, x_min, x_max, y_min, y_max , nullptr);

    // Left 2/5 of the map (from 0% to 40%)
    x_min = 0;
    x_max = 2 * MSZ / 5;

    // Middle vertically
    y_min = MSZ / 3;
    y_max = 2 * MSZ / 3;

    generateSquare(COMMANDER_2, &team2[0], 'C', 2, x_min , x_max , y_min, y_max , team2);
    generateSquare(WARRIOR_2_1, &team2[1], 'W', 2, x_min, x_max, y_min, y_max , nullptr);
    generateSquare(WARRIOR_2_2, &team2[2], 'W', 2, x_min, x_max, y_min, y_max , nullptr);
    generateSquare(MEDIC_2, &team2[3], 'M', 2, x_min, x_max, y_min, y_max , nullptr);
    generateSquare(LOGISTIC_2, &team2[4], 'P', 2, x_min, x_max, y_min, y_max , nullptr);
}

static void placeTrees()
{
    for (int t = 0; t < TREE_COUNT; t++)
    {
        int x = rand() % (MSZ - TREE_W);
        int y = rand() % (MSZ - TREE_H);

        if (map[x][y] == STONE) 
            continue;

        treeX[t] = x;
        treeY[t] = y;

        for (int j = 0; j < TREE_W; j++) 
        {
            for (int i = 0; i < TREE_H; i++) 
            {
                map[x + j][y + i] = TREE;
            }
        }
    }
}




void InitMap(NPC** team1, NPC** team2)
{
    // clear map
    for (int i = 0; i < MSZ; i++)
        for (int j = 0; j < MSZ; j++)
            map[i][j] = 0;

    placeStones();
    placeTrees();
    placeWater();
    placeArmoryMedicne();
    PlaceTroops(team1, team2);
}

void drawChar(int i, int j, int type, char character)
{
    if (map[i][j] == type &&
        map[i - 1][j] == type &&
        map[i + 1][j] == type &&
        map[i][j - 1] == type &&
        map[i][j + 1] == type)
    {
        glColor3d(0.0, 0.0, 0.0); // black text
        glRasterPos2d(j - 0.25, i - 0.5);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, character);

    }
}

void DrawMap()
{
    // Paint entire ground first (case 0)
    for (int i = 0; i < MSZ; i++)
    {
        for (int j = 0; j < MSZ; j++)
        {
            glColor3d(0.7, 1.0, 0.7); // ground
            glBegin(GL_POLYGON);
            glVertex2d(i, j);
            glVertex2d(i, j + 1);
            glVertex2d(i + 1, j + 1);
            glVertex2d(i + 1, j);
            glEnd();
        }
    }

    // Draw stones as grey squares over ground
    for (int i = 0; i < MSZ; i++)
    {
        for (int j = 0; j < MSZ; j++)
        {
            if (map[i][j] == STONE)
            {
                glColor3d(0.3, 0.3, 0.3);
                glBegin(GL_POLYGON);
                glVertex2d(i, j);
                glVertex2d(i, j + 1);
                glVertex2d(i + 1, j + 1);
                glVertex2d(i + 1, j);
                glEnd();
            }
        }
    }

    // Draw trees as ONE green triangle per tree (spanning 3x4 cells)
    for (int t = 0; t < TREE_COUNT; t++)
    {
        double x0 = treeX[t];
        double y0 = treeY[t];

        // Triangle that spans the 3x4 block:
        double baseLeftX = x0;
        double baseRightX = x0 + TREE_W;
        double baseY = y0;
        double apexX = x0 + TREE_W * 0.5;
        double apexY = y0 + TREE_H;

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
    }

    // Draw Water
    for (int i = 0; i < MSZ; i++)
    {
        for (int j = 0; j < MSZ; j++)
        {
            if (map[i][j] == WATER)
            {
                glColor3d(0.6, 0.8, 1.0);
                glBegin(GL_POLYGON);
                glVertex2d(i, j);
                glVertex2d(i, j + 1);
                glVertex2d(i + 1, j + 1);
                glVertex2d(i + 1, j);
                glEnd();
            }
        }
    }
    // Draw Hospital & Medicine
    for (int i = 0; i < MSZ; i++)
    {
        for (int j = 0; j < MSZ; j++)
        {
            if (map[i][j] == ARMORY_1 || map[i][j] == ARMORY_2 || map[i][j] == MEDICNE_1 || map[i][j] == MEDICNE_2)
            {
                glColor3d(1.0, 0.9, 0.2);
                glBegin(GL_POLYGON);
                glVertex2d(i, j);
                glVertex2d(i, j + 1);
                glVertex2d(i + 1, j + 1);
                glVertex2d(i + 1, j);
                glEnd();
            }
        }
    }








}