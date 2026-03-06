#pragma once
#include <string>

// Set to 1 to enable [SLOW] performance debug output (higher thresholds to reduce spam)
#ifndef DEBUG_SLOW
#define DEBUG_SLOW 0
#endif

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

const double BULLET_DAMAGE = 60.0;
const double GRENADE_BULLET_DAMAGE = MAX_HP * 0.01;

// Support units (Medic, Supply) flee / avoid when an enemy is visible within this distance
const double SUPPORT_AVOID_ENEMY_DIST = 22.0;

// Aspiration / state thresholds (spec)
const double HP_PANIC_RATIO = 0.25;       // HP below this -> escape to cover (GoToDefenseState)
const double HP_NEED_HEAL_RATIO = 0.5;   // HP below this -> target Medic (when not panic)
const double AMMO_NEED_RESUPPLY_RATIO = 0.2;  // Ammo below this -> target Supply
const double HP_RECOVER_RATIO = 0.4;     // HP above this at cover -> leave defense (MoveToTargetState)
const double HP_OK_RATIO = 0.9;          // Medic: stop healing when teammate reaches this (don't over-heal)
const int CHASE_NO_ENEMY_FRAMES = 30;   // Frames without visible enemy before leaving AttackState (chase or search)

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

// Maze / Room constants (actual count is random at runtime)
const int MAX_ROOMS = 20;
const int MAX_DEPOTS = 4;  // max armories + medicine depots each

// FPS: max pathfinding calls per frame (set by main each tick, consumed by NPC::PlanPathTo)
extern int g_pathFindBudget;
