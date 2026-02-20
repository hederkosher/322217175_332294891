#pragma once
#include <string>

// ANSI reset
const std::string RESET = "\033[0m";

// Team colors
const std::string TEAM1 = "\033[38;2;255;165;0m"; // Orange
const std::string TEAM2 = "\033[38;2;0;200;255m"; // Light Blue
const std::string GRENADE = "\033[38;2;180;255;0m";
const std::string GAME_END = "\033[38;2;180;0;255m"; // Royal purple

const double SPEED = 0.05;
const int MAX_HP = 1000;
const int AMMO_MAX = 100;
const int MEDICINE_MAX = 100;
const double SECURITY = 0.003;

const int TEAM_SIZE = 4; // 2 Warriors + 1 Medic + 1 Supply

const int MSZ = 100;

// Terrain types
const int FLOOR = 0;
const int WALL = 1;
const int STONE = 2;    // Cover obstacle in rooms
const int ARMORY = 3;   // Ammo depot
const int MEDICINE = 4; // Medicine depot

// NPC type IDs
const int WARRIOR_1_1 = 10;
const int WARRIOR_1_2 = 11;
const int WARRIOR_2_1 = 12;
const int WARRIOR_2_2 = 13;
const int MEDIC_1 = 14;
const int MEDIC_2 = 15;
const int SUPPLY_1 = 16;
const int SUPPLY_2 = 17;

// Maze / Room constants
const int GRID_ROWS = 3;
const int GRID_COLS = 3;
const int MAX_ROOMS = GRID_ROWS * GRID_COLS;
