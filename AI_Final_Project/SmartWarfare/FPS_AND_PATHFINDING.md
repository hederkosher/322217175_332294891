# FPS and A* pathfinding in Smart Warfare

## How FPS works in this game

### Timer-driven game loop

- **Target:** `TARGET_FPS = 160` (in `main.cpp`), so the game aims for 160 frames per second.
- **Frame interval:** `FRAME_MS = 1000 / TARGET_FPS` ≈ **6.25 ms** per frame.
- **Driver:** GLUT’s `glutTimerFunc(FRAME_MS, OnTimer, 0)`. Every ~6.25 ms the OS fires a timer; the game runs one **logic tick** in `OnTimer()` and then reschedules the next timer. So the **simulation** is fixed-rate (one tick per timer fire).
- **Rendering:** At the end of each tick the game calls `glutPostRedisplay()`, which tells GLUT to call the display callback when it can. So **FPS** = how many times per second the display runs, which in practice is limited by:
  1. How long each tick takes (if a tick takes > 6.25 ms, the next timer fires late and FPS drops).
  2. VSync (if enabled, FPS is capped to monitor refresh; the code disables it on Windows so FPS can go above refresh).

### What runs each tick (in order)

1. Death checks, win/timer checks.
2. **`g_pathFindBudget = 1`** — reset pathfinding budget (see below).
3. Visibility update for all 8 NPCs.
4. Security map update for combat rooms.
5. **AI update:** `DoSomeWork()` for Medic + Supply (both teams), then Warriors (both teams). This is where almost all **A* calls** happen.
6. Bullet and grenade movement.
7. `glutPostRedisplay()` + reschedule timer.

### How “FPS” is shown on screen

- In the display callback, the game counts frames over 1-second windows using `glutGet(GLUT_ELAPSED_TIME)` and shows “FPS: N” in the corner. That number is **display FPS**: how often the screen is redrawn per second. If one logic tick is very slow, the next display happens late, so the measured FPS drops.

So: **FPS is high when each tick (especially AI + pathfinding) finishes in well under ~6.25 ms.** If A* (or other work) makes a single tick take 20–30 ms, you get only ~33–50 “frames” per second in practice.

---

## How A* is used and why it’s heavy

### Where pathfinding is requested

Every unit that needs to move somewhere calls:

- **`PlanPathTo()`** — A* with NPC avoidance (other units as obstacles) and risk-weighted cost (security map). Uses the overload `FindPath(si, sj, ti, tj, path, myTeam, enemyTeam, this)`.
- **`PlanPathToIgnoreNPCs()`** — same A* but no NPC blocking (used when the first call fails or when the game wants a cheaper fallback). Uses `FindPath(si, sj, ti, tj, path)`.

So a single “I need a path” can trigger **one or two** full A* runs (try with NPCs, then maybe without).

Who calls these (examples):

- **Warriors:** going to medic/supply, to last-known enemy, search rooms, repath to medic when stuck, etc.
- **Medic:** GoToMedicine, GoToTarget (injured teammate), fleeing to warrior.
- **Supply:** GoToArmory, GoToWarrior, fleeing to warrior.
- **States:** GoToMedicine, GoToArmory, GoToTarget, GoToWarrior, GoToDefenseState (BFS, not A*), AttackState (chase), etc.

So in a single tick, **many** units can *want* to call `PlanPathTo()` or `PlanPathToIgnoreNPCs()`.

### Why each A* call is expensive

- **Map size:** Grid is **MSZ×MSZ = 100×100** cells. A* explores cells in “wavefront” order; long paths can touch hundreds of cells.
- **Per-cell work:** For each expanded cell, A*:
  - Pops from a priority queue.
  - Checks 8 neighbours.
  - For each neighbour: footprint check (3×3), diagonal blocking, **security map read** for risk cost, g/f update, push to queue.
- **NPC-avoidance version:** The overload that takes `team1`, `team2`, `exclude` uses a **footprint check that iterates over all living NPCs** for every candidate cell. So cost scales with number of units and with number of cells expanded.
- **Expansion cap:** Each A* run is limited to **`MAX_ASTAR_EXPANSIONS = 1200`** (in `AStar.cpp`). So one call can do up to 1200 iterations (priority_queue pop + 8 neighbours + security + NPC checks). That’s already on the order of tens of thousands of operations per path; with 100×100 and obstacles, some paths will hit the cap.
- **Two A* per “logical” path:** `PlanPathTo()` can call `FindPath` twice (with NPCs, then without), so one “plan path” can be **two** full A* runs.

So: **one frame with several path requests can easily do 2–3+ A* runs, each doing up to 1200 expansions with heavy per-cell work** → that’s the main reason a single tick can take 20–30 ms and **FPS drops**.

---

## How the game limits A* to protect FPS: path budget

### Global budget

- **Variable:** `g_pathFindBudget` (in `Definitions.h` / `NPC.cpp`). Main sets it to **1** at the start of each tick:  
  **`g_pathFindBudget = 1`** (comment: “1 path per frame caps cost (~25-30ms max); support runs first for spawn”).
- **Usage:**  
  - `PlanPathTo()`: if `g_pathFindBudget <= 0` it **returns false immediately** (no A*). Otherwise it decrements the budget and runs A* (and possibly a second A* fallback).  
  - `PlanPathToIgnoreNPCs()`: same — check budget, decrement, then run A*.

So **at most one “path request” is honoured per frame** (and that one request can still be 1 or 2 A* runs inside `PlanPathTo`). All other units that call `PlanPathTo()` or `PlanPathToIgnoreNPCs()` in that same tick get **no path** and keep their old path or “no path” state.

### Who gets the budget

- Update order is: **Medic and Supply first** (both teams), **then Warriors**.
- So when budget is 1, usually the **first** unit that actually calls `PlanPathTo()` in that frame (typically a support) gets the path; warriors often get no new path that frame and will retry on later frames when they happen to be “first” in the update order.

So **heavy A* usage is reduced by only allowing a small number of pathfinding calls per frame**. The downside: with budget 1, many units wait many frames before they get a new path, which can look like sluggish or “stuttery” movement and delayed reactions.

---

## Summary: why heavy A* drops FPS

1. **FPS** = how many times per second the game can complete a full tick (logic + render). Each tick is driven by a timer every **~6.25 ms**.
2. **A* is the costliest part of a tick:** each run can do up to **1200 node expansions** on a 100×100 grid, with priority queue, 3×3 footprint, security map, and (in one overload) NPC checks per neighbour.
3. **Multiple units** want to path every frame (medics, supply, warriors to medic/supply/enemy/rooms). Without a limit, you could get **many** A* runs per frame (e.g. 5–10+), each taking several ms → total tick time **>> 6.25 ms** → FPS drops (e.g. to 20–40).
4. **Path budget (`g_pathFindBudget = 1`)** caps how many path requests are served per frame (effectively 1), so worst case is about **1–2 A* runs per frame**. That keeps the tick time and FPS more stable but makes path updates rare for most units and can make movement feel delayed when many need paths at once.

To improve FPS further you could: lower `MAX_ASTAR_EXPANSIONS`, reduce grid resolution for pathfinding, cache/reuse paths for a few frames, or increase budget slightly and accept a bit more frame time variance.
