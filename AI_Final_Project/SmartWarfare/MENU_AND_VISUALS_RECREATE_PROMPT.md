# Prompt: Recreate Menu, Starting Screen, Visual Colors & Bullet Trails

Use this document as a specification to implement or recreate the game’s **menu**, **starting screen**, **global color scheme**, and **bullet trail** visuals in the Smart Warfare project.

---

## 1. Tech stack & coordinate system

- **Rendering**: OpenGL 2D via **GLUT** (`glut.h`).
- **Window**: 900×600 pixels, title `"Smart Warfare"`.
- **Projection**: `glOrtho(0, 100, 0, 100, -1, 1)` — world space is **0–100** in X and Y. All positions and sizes below are in this space unless noted.
- **Drawing order**: Clear → Map → NPCs → Bullets → Grenades → UI overlay → Starting screen overlay (if not started) or game-over text (if ended) → `glutSwapBuffers()`.

---

## 2. Starting screen (pre-game menu)

**When**: Shown when `gameStarted == false` (before the player presses a key).

**Layout**:

1. **Full-screen overlay**  
   - A single quad covering the whole viewport: from `(0,0)` to `(100,100)`.  
   - Color: dark blue-black, e.g. `glColor3d(0.02, 0.02, 0.08)` so the game view underneath is barely visible.

2. **Game logo (ASCII art)**  
   - Draw the “Smart Warfare” text logo (multi-line ASCII art, 5 lines).  
   - Use a **monospace / small bitmap font** (e.g. `GLUT_BITMAP_9_BY_15`).  
   - Position: roughly top-center in world space, e.g. `drawLogo(15, 78, font)` so the logo sits above center.  
   - Color: bright cyan/teal for visibility, e.g. `glColor3d(0.0, 0.9, 1.0)`.

3. **Call-to-action text**  
   - One line: **“Press any key to start”**.  
   - Font: larger, e.g. `GLUT_BITMAP_TIMES_ROMAN_24`.  
   - Position: center of screen in world coords, e.g. `drawText(36, 48, "Press any key to start", font)`.  
   - Same accent color as logo (e.g. cyan) or white for clarity.

**Interaction**: On any key press (in the keyboard callback), set `gameStarted = true` and start the match timer so the overlay is no longer drawn and gameplay begins.

---

## 3. In-game menu (right-click)

- **Type**: GLUT right-click context menu.  
- **Attach**: `glutAttachMenu(GLUT_RIGHT_BUTTON)`.  
- **Entries**:  
  - **“Show Security Map”** — switch display to security/risk map view.  
  - **“Show Regular Map”** — switch back to normal game view.  
- **Behavior**: Menu callback receives the chosen entry id and sets the current “view mode” (e.g. `glutDisplayFunc(display)` for game, `glutDisplayFunc(displaySecurityMap)` for security map), then `glutPostRedisplay()`.

---

## 4. Visual color scheme

Use a consistent palette so the game is readable and teams are distinguishable.

### 4.1 Teams (NPCs, bullets, UI)

| Element        | Team 1 (Red)     | Team 2 (Cyan)     |
|----------------|------------------|-------------------|
| Primary        | Orange-red       | Cyan              |
| RGB example    | `1.0, 0.5, 0.0`  | `0.0, 0.8, 1.0`   |
| Darker variant | `1.0, 0.1, 0.1`  | e.g. dimmer cyan  |
| Use            | Warriors, bullets, team UI | Same for team 2 |

- **Warrior 1**: red tint `(1.0, 0.1, 0.1)`.  
- **Warrior 2**: orange tint `(1.0, 0.8, 0.0)`.  
- **Medic**: green tint `(0.0, 1.0, 0.4)`.  
- **Supply**: re-use team base color or a neutral (e.g. gray/white) for the pack icon.

### 4.2 UI and overlays

| Element           | Color (R,G,B)        | Purpose                    |
|-------------------|----------------------|----------------------------|
| Background clear  | `0.01, 0.01, 0.04`   | Slightly blue-black        |
| Start overlay     | `0.02, 0.02, 0.08`   | Dark overlay for start     |
| Logo / accent     | `0.0, 0.9, 1.0`      | Logo and “press key”       |
| Timer (normal)    | `1.0, 1.0, 1.0`      | White                      |
| Timer (urgent)    | `1.0, 0.2, 0.2`      | Red when time low          |
| “Time’s up”       | `1.0, 0.8, 0.0`      | Yellow                     |
| Win message       | `1.0, 1.0, 1.0`      | White                      |
| Hint (e.g. R)     | `0.7, 0.7, 0.7`      | Gray                       |
| FPS counter       | `0.0, 1.0, 0.5`      | Green                      |

### 4.3 Map and environment

- **Floor / passage**: dark blue-gray (e.g. `0.04–0.12` range).  
- **Walls**: darker (`0.02–0.06`).  
- **Room tint**: slight purple or brown tint per room type.  
- **Obstacles (e.g. stone)**: neutral gray-brown.  
- **Armory**: warm yellow/gold (`0.9, 0.75, 0.1`).  
- **Medicine depot**: red cross / red tint (`0.9, 0.2, 0.3` or similar).  
- **Room outlines**: thin lines in cyan/blue (`0.0, 0.7, 1.0`) for clarity.

### 4.4 NPC display

- **Body**: Small quad (e.g. 1×1 unit) with team color; optional **glow**: draw a slightly larger, dimmer quad behind (e.g. `0.3 * teamColor`).  
- **Symbol**: White character on top (e.g. `glutBitmapCharacter`) for W/M/P.  
- **HP bar**: Short line or bar above the unit; color from red (low) to green (high), e.g. `(1 - normalizedHP, normalizedHP, 0.2)`.

---

## 5. Bullet trails

**Goal**: Each bullet draws a short trail behind it so the shot direction and team are visible.

### 5.1 Data

- **Trail storage**: Ring buffer of recent positions (e.g. `TRAIL_LEN = 16` or 24).  
- **Arrays**: `trailX[]`, `trailY[]`, one index `trailIdx`, and `trailCount` (number of valid samples, up to `TRAIL_LEN`).  
- **Update**: Each frame the bullet moves, store current `(x, y)` at `trailIdx`, then advance `trailIdx = (trailIdx + 1) % TRAIL_LEN` and cap `trailCount`.

### 5.2 Drawing (order and style)

1. **Draw trail first** (behind the bullet head).  
2. **Segment by segment**: For each consecutive pair of stored positions, draw a **line segment**.  
   - **Color**: Same as bullet team color (Team 1: orange-red, Team 2: cyan), but **fade by age**: e.g. `fade = 1.0 - (double)i / trailCount` so older segments are dimmer.  
   - **Line width**: Slightly thicker than 1.0; can scale with fade, e.g. `2.5 * fade + 0.5`, then reset to `1.0` after the trail.  
3. **Bullet head**: Draw a small diamond or quad centered at current `(x, y)` in full team color (no fade).  
   - Size: `drawRadius` (e.g. 0.15–0.3 in world units).  
   - Shape: e.g. four vertices `(x±r, y)` and `(x, y±r)` (diamond).

### 5.3 Optional enhancements

- **Grenades**: Same trail idea with a different color (e.g. yellow/orange) and possibly a larger “head” or explosion sprite.  
- **Max trail length**: Keep `TRAIL_LEN` modest (e.g. 16–24) so trails don’t dominate the screen and performance stays good.

---

## 6. Ammo & health packs (visual + brief behavior)

### 6.1 Armory / ammo packs

- **Purpose**: Static locations where the Supply unit refills its ammo, and indirectly where warriors get resupplied via the Supply.  
- **Icon & shape**:  
  - Base shape: stylized **ammo crate / box** near the floor, drawn as stacked quads around the armory cell.  
  - Overlay: small horizontal **bullet icon** or simple bar to imply ammunition.  
- **Color**:  
  - Main: warm gold/yellow (`0.9, 0.75, 0.1`) for the arrow/marker.  
  - Box body: brown/orange range (`0.45–0.85` in red/green, low blue) to feel like metal/wood.  
- **Placement**:  
  - Positioned at predefined armory cells on the map; typically one or more per team.  
  - Drawn **on top of floor tiles**, but below NPCs and bullets.  
- **Behavior summary (for UI)**:  
  - Supply NPCs move to the armory, enter a **FillAmmo** state, and refill their internal ammo pool.  
  - While filling, you can show a small progress cue above the Supply (bar or pulsing glow) but the **armory itself** is static and does not animate heavily (cheap to draw).\n
### 6.2 Medicine depots / health packs

- **Purpose**: Static locations where the Medic refills its medicine stock; they indirectly produce “heals” rather than movable health items.  
- **Icon & shape**:  
  - Simple **white square / panel** on the floor with a bold **red cross** on top.  
  - Cross can be drawn by two overlapping quads (vertical and horizontal bar).  
- **Color**:  
  - Panel: light gray/white (`0.95, 0.95, 0.97`).  
  - Cross: vivid red (`0.9, 0.2, 0.3`).  
- **Placement**:  
  - At predefined medicine depot cells; usually in safe-ish rooms or near the back line.  
  - Like armories, drawn above floor but below NPCs and bullets.  
- **Behavior summary (for UI)**:  
  - Medic NPCs path to these depots and enter **FillMedicine**; while in that state they refill their medicine resource.  
  - If you want extra feedback, you can pulse the cross color or draw a small green bar above the medic while filling; the **depot icon remains static**.

## 7. Summary checklist for implementation

- [ ] **Starting screen**: Full-screen dark overlay quad when `!gameStarted`; logo at top-center; “Press any key to start” at center; key handler sets `gameStarted = true`.  
- [ ] **Menu**: Right-click GLUT menu with “Show Security Map” and “Show Regular Map”; switch `glutDisplayFunc` and redraw.  
- [ ] **Colors**: Apply the table above for teams, UI, map, NPCs, and static props (armory, medicine).  
- [ ] **Bullet trails**: Ring buffer of positions updated in bullet `Move()`; in `Show()`, draw fading line segments then bright bullet head in team color.  
- [ ] **Drawing order**: Map → static props (armory + medicine) → NPCs → Bullets (with trails) → Grenades → UI (timer, FPS, team stats) → Game-over or start overlay.

This prompt is self-contained so that someone (or an AI) can recreate the menu, starting screen, visual style, bullet trails, and visual treatment of ammo / health packs without reading the rest of the codebase in detail.
