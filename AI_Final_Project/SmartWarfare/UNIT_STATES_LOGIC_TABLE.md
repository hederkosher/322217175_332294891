# Unit states & logic — rework reference

Use this table to rework unit logic: first column is the state, second is what is implemented now, third is left for your planned changes.

---

## Warrior

| State | Implemented logic (current) | Your notes / new logic |
|-------|----------------------------|------------------------|
| **MoveToTargetState** | Default state. Move along A* path toward current target. Target can be: enemy (from SearchForEnemies), medic (low HP), or search room. Transition: if not in risk and enemy in same room and ammo > 0 → AttackState. Personality (aggressiveness/cautiousness) drives flee thresholds. | **Purpose:** Execute the currently selected aspiration target (enemy / medic / supply / search-room). **Path planning:** Use A*; if the unit is in or heading into a room with active combat, use a risk-weighted cost (safety map) so paths bias away from danger hotspots. **Target selection (high → low):** (1) If HP < HP_panic → EscapeToCover (GoToDefenseState). (2) Else if HP < HP_needHeal → target Medic. (3) Else if ammo < Ammo_needResupply → target Supply/Armory flow. (4) Else if enemy known → pursue last-known enemy room; if unknown → search (room sweep pattern). **Room entry rule:** While in corridor/transition, don’t fight; combat only triggers once inside same room (passages for movement, not combat). **Transitions:** If enemy in same room + LOS + ammo>0 → AttackState. If target reached and it’s “search-room” → pick next search-room (continue MoveToTarget). |
| **AttackState** | Stop moving, set isAttacking; shoot at enemy in same room (LOS). Transition: if ammo ≤ 0 → MoveToTargetState; if no enemy for 30 frames → MoveToTargetState (optionally chase last known position). | **Purpose:** Engage enemy when co-located in same room. **Fire mode:** Choose between (a) single bullet, (b) grenade: grenade if (enemy behind cover OR ≥2 enemies in blast radius OR enemy very close) and self at safe distance; otherwise single bullet. **Positioning:** Micro-reposition: if LOS blocked by obstacle/cover, step to a nearby “peek” point (same room) using local A* with safety weights. **Transitions:** ammo==0 → MoveToTargetState (resupply). HP < HP_panic → GoToDefenseState. No enemy visible for N frames → MoveToTargetState toward last-known enemy position/room (chase), else resume search. |
| **GoToDefenseState** | When low HP/ammo (and not going to medic): BFS to nearest cover, follow path. On arrival: stay at cover (no transition to Idle). Never shoot while in this state. | Rework into true **“Survive/Escape”** aspiration. **Behavior:** Pick best cover point in current room (or adjacent safe room if current room too dangerous) using BFS/A* where cost = distance + risk (safety map). **At cover:** “Hold/peek” loop—stay behind obstacle; occasionally peek if aggressiveness high and HP above threshold; otherwise stay hidden until recovered/help arrives. **Optional:** Allow shooting from defense if cautiousness low AND LOS from cover; otherwise keep “no shooting” for simpler behavior. **Transitions:** HP > HP_recover and ammo ok → MoveToTargetState. If medic reachable and HP still low → MoveToTargetState toward medic. |
| **IdleState** | Referenced in code (e.g. grenade throw when “idle”) but never explicitly entered via setCurrentState; treated as “in moving state” for some checks. OnExit for Supply sets isMoving true. | Make it an **explicit state** used only when “arrived and waiting” (e.g. at cover holding, or waiting for supply/medic). **Entry:** Reached destination and no immediate action (e.g. waiting for teammate). **Exit:** Any new higher-priority aspiration → setCurrentState to MoveToTarget / GoToDefense / Attack. |

*Warrior priority order in DoSomeWork: (1) critical HP → move to medic or GoToDefenseState, (2) low ammo → move to supply or GoToDefenseState, (3) grenade evasion, (4) SearchForEnemies / room-based target, (5) attack if enemy in room, (6) reposition when shots blocked.*

**Warrior personality knobs:** *aggressiveness* → grenade usage threshold, chase time, peek-from-cover. *cautiousness* → HP_panic, risk-weight in path cost, willingness to fight while hurt.

---

## Medic

| State | Implemented logic (current) | Your notes / new logic |
|-------|----------------------------|------------------------|
| **GoToMedicine** | Path to closest free medicine depot (team-aware occupancy). OnEnter: set target, PlanPathTo. Transition: on arrival → FillMedicine. | Keep: go to closest free depot (team occupancy). **Add:** If combat active in current/target room, route using safety-map weights (avoid danger). If enemy appears in same room → “flee toward nearest warrior” override remains, but also prefer moving behind cover obstacles during flee. |
| **FillMedicine** | Set isFillingMedicine, occupy depot; actual refill done in main/update. Transition: → GoToTarget. OnExit: release depot, clear isFillingMedicine. | Keep occupancy + refill. **Add:** If enemy enters same room while filling → immediately abort (release depot) and flee toward warrior/cover (GoToTarget with closest warrior OR dedicated “SupportDefense” path). |
| **GoToTarget** | Path to current injured teammate (pTarget). If no target, FindInjuredTeammate and set pTarget. Transition: on arrival → GiveMedicine. | Rework target selection: choose injured teammate by **score = (missingHP × weight) − (risk × weight) − (distance × weight)** to avoid medic suiciding into hot rooms. **Add:** When approaching target in a fight room, path to a safe adjacent cover point near teammate (not exact position), then step in to heal when safe (short dash). |
| **GiveMedicine** | Stop moving, set isGivingMedicine, target set isGettingHp. Transition: → GoToMedicine. OnExit: clear isGivingMedicine, goToTarget, target isGettingHp false. | Keep: stop + heal. **Add:** Healing can be interrupted if risk spikes (enemy close/LOS) → abort heal, flee/retreat to cover, then retry. Don’t over-heal—once teammate reaches HP_ok, release and re-evaluate next injured or return to depot. |

*Medic overrides: when in risk or enemy visible within SUPPORT_AVOID_ENEMY_DIST, flee toward FindClosestWarrior (switch to GoToTarget with warrior as target). When target dead or fully healed, clear target and → GoToMedicine. Autonomous scan (scanCooldown) picks FindInjuredTeammate and switches to GoToTarget.*

---

## Supply

| State | Implemented logic (current) | Your notes / new logic |
|-------|----------------------------|------------------------|
| **GoToArmory** | Path to closest free armory (team-aware occupancy). OnEnter: set target, PlanPathTo. Transition: on arrival → FillAmmo. | Keep: closest free armory + A*. **Add:** Safety-map pathing in combat rooms. If enemy in same room → flee toward closest warrior/cover. |
| **FillAmmo** | Set isFillingAmmo, occupy depot; refill in update. Transition: → GoToWarrior. OnExit: release depot, clear isFillingAmmo. | Keep refill + occupancy. **Add:** Abort if enemy enters room; don’t die at depot—release and flee. |
| **GoToWarrior** | Path to pWarrior (or FindWarriorNeedingAmmo / FindWarriorWithLowestAmmo). Transition: if warrior alive and ammo < 95% → GiveAmmo; else clear warrior and → GoToArmory. | Rework warrior selection: **score = (1 − ammoRatio) × priority − distancePenalty − riskPenalty**. Prefer warriors actively fighting (same room as enemy) only if route risk acceptable; otherwise service safer warrior first. **Add:** Approach to a safe meeting point (cover-adjacent) instead of walking into open LOS. |
| **GiveAmmo** | Stop moving, set isGivingAmmo. Transition: → GoToArmory. OnExit: clear isGivingAmmo, goToWarrior. | Keep: stop + refill warrior. **Add:** Can be interrupted by risk spike; if interrupted, retreat to cover then retry for a few seconds; otherwise abandon and return to armory. |

*Supply overrides: when stuck too long going to warrior, clear warrior and → GoToArmory. When in risk or enemy visible within SUPPORT_AVOID_ENEMY_DIST, flee toward FindClosestWarrior (switch to GoToWarrior with that warrior). When warrior dead or ammo ≥ 95%, optionally find next warrior needing ammo or → GoToArmory.*

---

## Small global adjustments (FSM reads like the spec)

- **Explicit aspiration selection** every tick (or every few frames): pick one goal at a time (fight / survive / resupply / heal / search).
- **Combat only when both in same room;** corridors are movement-only.
- **Safety map integration:** whenever anyone in a fight room moves, update that room’s safety map and re-plan paths for units whose next steps are inside that room.
- **Obstacles as cover:** define “cover points” behind obstacles; let Warriors/Supports path to them when fleeing/meeting.

---

## State flow (current)

- **Warrior:** MoveToTargetState ⇄ AttackState; MoveToTargetState → GoToDefenseState (flee); medic/supply targets use MoveToTargetState with path to teammate.
- **Medic:** GoToMedicine → FillMedicine → GoToTarget → GiveMedicine → GoToMedicine (loop).
- **Supply:** GoToArmory → FillAmmo → GoToWarrior → GiveAmmo → GoToArmory (loop).
