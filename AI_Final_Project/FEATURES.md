# Smart Warfare - Main Features

## 1. AI-Driven Squad Combat
Two teams of 4 AI-controlled soldiers (2 Warriors, 1 Medic, 1 Supply) battle autonomously in a procedurally generated dungeon. Each unit uses a Finite State Machine (FSM) to make independent decisions — attacking enemies, fleeing when injured, seeking resupply, or hunting targets across rooms.

## 2. Procedural Map Generation
Every match generates a unique dungeon layout with randomized rooms, corridors, cover obstacles (stones), and single-instance resource depots (one ammo pack, one health pack). No two matches play out the same way.

## 3. A* Pathfinding with Dynamic NPC Avoidance
Units navigate the map using A* pathfinding that accounts for other NPCs as dynamic obstacles. A multi-tier fallback system (avoidance-aware A* → basic A* → escape moves) ensures soldiers never get permanently stuck, even in tight corridors.

## 4. Smart Shooting & Wall-Aware Repositioning
Warriors trace the path of each bullet before firing. If the shot would hit a wall corner or obstacle before reaching the enemy, the warrior withholds fire, tracks consecutive blocked shots, and automatically repositions laterally to find a clear line of fire. This prevents wasted ammo on wall edges and creates more realistic flanking behavior.

## 5. Role-Based Unit Classes
- **Warriors** — Front-line fighters that search rooms, engage enemies with bullets and grenades, flee to cover when HP is critical, and reposition when their line of fire is blocked by walls.
- **Medics** — Support units that detect injured teammates, path to them, and restore HP. They flee to cover when under fire.
- **Supply** — Logistics units that travel to the ammo depot to resupply, then deliver ammunition to warriors running low. They prioritize the most depleted teammate.

## 6. Grenade System with Parabolic Flight
Warriors throw grenades when multiple enemies cluster in a room. Grenades travel along a visible parabolic arc from the thrower to the target, land with a blinking fuse timer, then detonate into a radial shrapnel burst that damages all nearby units.

## 7. Tron-Neon Visual Style
The entire game uses a dark Tron-inspired color palette: neon cyan room outlines, glowing NPC bodies with colored halos, neon bullet trails that fade behind projectiles, team-colored UI elements, and hand-drawn depot icons (bullet cartridges for ammo, white box with red cross for health).

## 8. Real-Time Match Timer & Win Conditions
Each match runs on a 1-minute countdown timer displayed on screen. The match ends when the timer expires or an entire team is eliminated. If the timer runs out, the winner is determined by comparing total remaining HP across both teams.

## 9. Live Team Statistics HUD
Both top corners of the screen display real-time stats for each team: individual soldier HP bars, ammo counts, and role labels. HP bars use a neon red-to-green gradient, and ammo/medicine bars shift from red through yellow to green based on remaining supply.
