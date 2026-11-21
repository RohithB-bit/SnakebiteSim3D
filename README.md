# 🐍 SnakebiteSim3D — 3D Interactive Snakebite Emergency & First-Aid Simulator

An interactive, 6-stage 3D medical emergency training simulator built from scratch in **C++17** and **OpenGL 1.1 (FreeGLUT)** with **zero external asset files**. Every 3D mesh, texture, particle effect, and sound effect is procedurally generated in C++ RAM at runtime.

---

## ✨ Key Features

* **100% Procedural 3D Graphics:** Continuous 24-segment lofted `GL_QUAD_STRIP` cobra mesh with flared hood and fangs, articulated human characters with walk cycles, 3D ambulance with flashing beacons, and five 128×128 procedural textures (grass, asphalt, hospital tiles, reptile scales, bark).
* **Procedural PCM Audio Engine:** Non-blocking synthesized WAV audio via Windows `winmm` (ECG heartbeat monitor, snake strike warning, clinical chimes/buzzers, ambulance siren) with `[M]` mute toggle.
* **Medically Accurate Clinical Protocol:** Real-time venom progression, BPM/ECG vitals monitoring, myth-busting penalties (tourniquets, cutting/sucking), and Polyvalent Anti-Snake Venom (ASV) IV management.
* **Faculty / Demo Mode:** Right-click context menu allows instant jumping to any stage (0–6) or camera preset.

---

## 🎮 Playable Stages

1. **Stage 0 — Title Showcase:** Orbiting illuminated 3D cobra and medical cross showcase.
2. **Stage 1 — Forest Trail & Ambush:** Walk the trail (`WASD`), encounter the slithering cobra, and identify venomous vs. non-venomous protocol.
3. **Stage 2 — Immediate First Aid:** Interactive **Splint Tension Meter** mini-game (`SPACE` in the green zone) to immobilize the limb without causing tourniquet necrosis.
4. **Stage 3 — Stretcher Evacuation:** Load and carry the patient (`WASD`) smoothly to the glowing extraction zone and parked ambulance.
5. **Stage 4 — Highway Ambulance Dodge:** 3-lane arcade driving (`A`/`D`), siren boost, and combo scoring while rushing to the ER.
6. **Stage 5 — Hospital ER & ASV Infusion:** Regulate the **IV Drip Flow Rate** (`W`/`S`) inside the therapeutic green zone to neutralize venom.
7. **Stage 6 — Clinical Debrief:** S / A / B / C / F ranking with full clinical decision audit log and instant replay.

---

## ⌨️ Controls

| Input | Action |
| :--- | :--- |
| **`WASD` / Arrow Keys** | Move character (Stages 1–3), Steer ambulance `A`/`D` (Stage 4), Adjust IV drip `W`/`S` (Stage 5) |
| **Mouse Left-Drag / Scroll** | Orbit 3D camera / Zoom in & out |
| **`[1]` – `[4]`** | Execute clinical decisions & actions |
| **`[SPACE]`** | Lock Splint Tension Meter in green zone (Stage 2) |
| **`[E]`** | Approach / inspect trigger (Stage 1) |
| **`[C]`** | Cycle camera presets |
| **`[M]`** | Mute / Unmute procedural audio |
| **`[R]`** | Restart simulation |
| **Right-Click** | Open stage-jump & camera context menu |
| **`[ESC]`** | Exit simulator |

---

## 🚀 Quick Start (Windows)

1. Click the green **`<> Code`** button above and select **Download ZIP**.
2. **Extract** the ZIP file to a folder on your PC *(do not run directly from inside the ZIP preview)*.
3. Open the extracted folder and double-click **`SnakebiteSim3D.exe`**.
   * *If Windows SmartScreen appears, click **More info → Run anyway**.*

---

## 🛠️ Building from Source

Requires **MSYS2 (MinGW-w64)** with `g++` and `freeglut`:

```bash
g++ -O2 -std=c++17 main.cpp -o SnakebiteSim3D.exe -static-libgcc -static-libstdc++ -lfreeglut -lglu32 -lopengl32 -lwinmm -lgdi32
