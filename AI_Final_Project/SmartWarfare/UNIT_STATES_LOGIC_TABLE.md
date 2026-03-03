# Unit states & logic — rework reference

Use this table to rework unit logic: first column is the state, second is what is implemented now, third is left for your planned changes.

---

## Warrior

| State | Implemented logic (current) | Your notes / new logic |
|-------|----------------------------|------------------------|
| **MoveToTargetState** | Default state. Move along A* path toward current target. Target can be: enemy (from SearchForEnemies), medic (low HP), or search room. Transition: if not in risk and enemy in same room and ammo > 0 → AttackState. Personality (aggressiveness/cautiousness) drives flee thresholds. | |
| **AttackState** | Stop moving, set isAttacking; shoot at enemy in same room (LOS). Transition: if ammo ≤ 0 → MoveToTargetState; if no enemy for 30 frames → MoveToTargetState (optionally chase last known position). | |
| **GoToDefenseState** | When low HP/ammo (and not going to medic): BFS to nearest cover, follow path. On arrival: stay at cover (no transition to Idle). Never shoot while in this state. | |
| **IdleState** | Referenced in code (e.g. grenade throw when “idle”) but never explicitly entered via setCurrentState; treated as “in moving state” for some checks. OnExit for Supply sets isMoving true. | |

*Warrior priority order in DoSomeWork: (1) critical HP → move to medic or GoToDefenseState, (2) low ammo → move to supply or GoToDefenseState, (3) grenade evasion, (4) SearchForEnemies / room-based target, (5) attack if enemy in room, (6) reposition when shots blocked.*

---

## Medic

| State | Implemented logic (current) | Your notes / new logic |
|-------|----------------------------|------------------------|
| **GoToMedicine** | Path to closest free medicine depot (team-aware occupancy). OnEnter: set target, PlanPathTo. Transition: on arrival → FillMedicine. | |
| **FillMedicine** | Set isFillingMedicine, occupy depot; actual refill done in main/update. Transition: → GoToTarget. OnExit: release depot, clear isFillingMedicine. | |
| **GoToTarget** | Path to current injured teammate (pTarget). If no target, FindInjuredTeammate and set pTarget. Transition: on arrival → GiveMedicine. | |
| **GiveMedicine** | Stop moving, set isGivingMedicine, target set isGettingHp. Transition: → GoToMedicine. OnExit: clear isGivingMedicine, goToTarget, target isGettingHp false. | |

*Medic overrides: when in risk or enemy visible within SUPPORT_AVOID_ENEMY_DIST, flee toward FindClosestWarrior (switch to GoToTarget with warrior as target). When target dead or fully healed, clear target and → GoToMedicine. Autonomous scan (scanCooldown) picks FindInjuredTeammate and switches to GoToTarget.*

---

## Supply

| State | Implemented logic (current) | Your notes / new logic |
|-------|----------------------------|------------------------|
| **GoToArmory** | Path to closest free armory (team-aware occupancy). OnEnter: set target, PlanPathTo. Transition: on arrival → FillAmmo. | |
| **FillAmmo** | Set isFillingAmmo, occupy depot; refill in update. Transition: → GoToWarrior. OnExit: release depot, clear isFillingAmmo. | |
| **GoToWarrior** | Path to pWarrior (or FindWarriorNeedingAmmo / FindWarriorWithLowestAmmo). Transition: if warrior alive and ammo < 95% → GiveAmmo; else clear warrior and → GoToArmory. | |
| **GiveAmmo** | Stop moving, set isGivingAmmo. Transition: → GoToArmory. OnExit: clear isGivingAmmo, goToWarrior. | |

*Supply overrides: when stuck too long going to warrior, clear warrior and → GoToArmory. When in risk or enemy visible within SUPPORT_AVOID_ENEMY_DIST, flee toward FindClosestWarrior (switch to GoToWarrior with that warrior). When warrior dead or ammo ≥ 95%, optionally find next warrior needing ammo or → GoToArmory.*

---

## State flow (current)

- **Warrior:** MoveToTargetState ⇄ AttackState; MoveToTargetState → GoToDefenseState (flee); medic/supply targets use MoveToTargetState with path to teammate.
- **Medic:** GoToMedicine → FillMedicine → GoToTarget → GiveMedicine → GoToMedicine (loop).
- **Supply:** GoToArmory → FillAmmo → GoToWarrior → GiveAmmo → GoToArmory (loop).

Fill the “Your notes / new logic” column with the behavior you want for each state when you rework the logic.
