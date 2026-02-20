#pragma once
#include <string>

// ANSI reset
const std::string RESET = "\033[0m";

// Team colors
const std::string TEAM1 = "\033[38;2;255;165;0m";   // Orange
const std::string TEAM2 = "\033[38;2;0;200;255m";   // Light Blue
const std::string GRENADE = "\033[38;2;180;255;0m";
const std::string GAME_END = "\033[38;2;180;0;255m"; // Royal purple

const double SPEED = 0.05;
const int MAX_HP = 1000;
const int AMMO_MAX = 100;
const int MEDICINE_MAX = 100;
const double SECURITY = 0.003;

const int TEAM_SIZE = 5;

const int MSZ = 100;
//Armory and Medicine positions
const int ARMORY_1_X = 90;
const int ARMORY_1_Y = 50;

const int ARMORY_2_X = 10;
const int ARMORY_2_Y = 60;

const int MEDICINE_1_X = 85;
const int MEDICINE_1_Y = 65;

const int MEDICINE_2_X = 1;
const int MEDICINE_2_Y = 43;


const int STONE = 1;
const int TREE = 2;
const int WATER = 3;
const int ARMORY_1 = 4;
const int ARMORY_2 = 5;
const int MEDICNE_1 = 6;
const int MEDICNE_2 = 7;
const int COMMANDER_1 = 8;
const int COMMANDER_2 = 9;
const int WARRIOR_1_1 = 10;
const int WARRIOR_1_2 = 11;
const int WARRIOR_2_1 = 12;
const int WARRIOR_2_2 = 13;
const int MEDIC_1 = 14;
const int MEDIC_2 = 15;
const int LOGISTIC_1 = 16;
const int LOGISTIC_2 = 17;





