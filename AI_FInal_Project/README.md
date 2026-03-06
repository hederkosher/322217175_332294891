# Smart WarFare

Short 2D tactical game: two teams (orange vs blue) of 4 units each (2 Warriors, 1 Medic, 1 Supply) fight on a grid map. Warriors attack and defend; Medics heal teammates; Supply units bring ammo and medicine from depots. **Win** by killing all enemy warriors, or by having more total HP when the 1‑minute match ends.

---

## How to run

1. Open **`Smart_WarFare.sln`** in Visual Studio.
2. **Build platform:** set solution to **x86** (Win32). **x64 does not run this project.**
3. **Preprocessor:** In project properties → **C/C++** → **Preprocessor** → **Preprocessor Definitions**, add:
   - `_CRT_SECURE_NO_WARNINGS`
   - (If you still get CRT warnings, add `_CRT_NO_WARNINGS` as well.)
4. Build (e.g. F7) and run. On the start screen, press any key or click to begin.
