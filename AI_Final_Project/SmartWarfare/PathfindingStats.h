#pragma once

// Per-frame pathfinding stats (set by main at start of tick, consumed by NPC/AStar)
extern int g_currentFrame;

// Set by main before each DoSomeWork(team, slot) so we know who requested a path
extern int g_currentPathRequesterTeam;
extern int g_currentPathRequesterSlot;

// Accumulated during the tick (AStar and NPC update these)
extern double g_pathfindingMs;
extern int g_astarCallsThisFrame;
extern int g_astarExpansionsThisFrame;

// Who actually got the last path (set by NPC when it calls FindPath)
extern int g_lastPathRequesterTeam;
extern int g_lastPathRequesterSlot;

// Per-unit replan cooldown: only allow PlanPathTo* every N frames
const int REPLAN_COOLDOWN_FRAMES = 10;

// Slow-frame threshold (ms): if tick exceeds this, print one-line summary
const double SLOW_FRAME_THRESHOLD_MS = 20.0;
