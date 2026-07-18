# 50-Minute Build Plan

## Product target

Create a compact top-down Windows arena survivor. The player moves while weapons fire automatically, survives growing enemy pressure, gains XP, chooses upgrades, dies, and can immediately restart.

## Technical decision

Use **C17 + Win32 API + GDI** and produce one native executable.

Why this stack fits the contest:

- Windows already provides the window, input, timing, drawing, and basic sound APIs.
- There is no bundled engine or runtime to consume the 1,474,560-byte allowance.
- Primitive shapes, particles, screen shake, and simple synthesized tones can provide readable game feel without external assets.
- A single executable is easy to measure and launch-test.

Preferred toolchain order: existing MSVC (`cl`), then installed LLVM/MinGW if already present. Do not download a toolchain during the timed run unless the environment has none and the user explicitly allows it.

## Intended repository structure

```text
AGENTS.md
PLAN.md
PACKAGE.md
TEST.md
RALPH_LOG.md          # created during the run
README.md             # created during the run
src/
  main.c              # game, Win32 shell, rendering; split only if useful
build.ps1
dist/
  Ralphthon.exe       # only required playable payload
```

## Milestones and acceptance criteria

### M0 — Native skeleton (00–05)

- Discover an installed compiler without changing the system.
- Create a release build command in `build.ps1`.
- Open a 960×540 resizable or fixed client window with a steady update/render loop.
- Escape or window close exits cleanly.

Acceptance: build exits 0 and `dist/Ralphthon.exe` launches as a native Windows process.

### M1 — Movement and pressure (05–15)

- Player movement with WASD and arrows; normalize diagonals.
- Clamp player to the arena.
- Spawn enemies beyond the play area and steer them toward the player.
- Remove far/dead entities and use fixed-capacity arrays.

Acceptance: 60 seconds of movement with continuous spawning and no crash or unbounded allocation.

### M2 — Combat and progression input (15–25)

- Auto-target nearest enemy and fire on cooldown.
- Resolve projectile/enemy hits and contact damage.
- Add HP, brief hit invulnerability, kills, XP, and level thresholds.
- Use simple flashes, trails, or particles for readable hits.

Acceptance: enemies can be killed, the player can be damaged, XP increases, and a level can be reached.

### M3 — Upgrade loop (25–35)

- Pause simulation on level-up.
- Offer two or three numbered choices.
- Implement at least three: damage, fire rate, projectile count/penetration, movement speed, max HP/heal.
- Allow upgrades to stack within safe caps.

Acceptance: selection resumes play and produces a visible or measurable stat change.

### M4 — Complete game and polish (35–42)

- HUD: HP, level/XP, time, kills.
- Difficulty rises through spawn cadence, enemy speed/HP, or enemy variants.
- Death freezes combat, reports result, and `R` restarts all state.
- Add cheap game feel: screen shake, hit flash, particles, pulsing shapes, synthesized beeps.

Acceptance: start → fight → level/upgrade → escalating survival → death → restart is connected.

### M5 — Stabilize (42–47)

- Run the manual play scenario in `TEST.md`.
- Fix blockers first, then major defects.
- Check entity caps, zero-length vector guards, timer bounds, and restart reset.
- Compile release settings and inspect imported DLLs.

Acceptance: no known blocker or major defect and package budget passes.

### M6 — Ship (47–50)

- Freeze features.
- Delete accidental payloads from `dist/`.
- Clean-build, measure recursively, launch the exact packaged EXE, and record evidence.
- Finish concise controls/build notes in `README.md` outside `dist/` unless rules require inclusion.

Acceptance: all release gates in `TEST.md` pass and `dist/` totals at most 1,474,560 bytes.

## Detail backlog

Only pull from this list after the full loop works. Favor improvements that cost little code and no asset bytes.

- Enemy archetypes distinguished by color, radius, speed, and HP.
- Elite enemy on a timer.
- Pickup magnetism and combo-like kill feedback.
- Directional particles, damage flashes, shadows, and screen shake.
- Procedural grid/starfield background and vignette.
- Weapon evolution or one secondary weapon.
- Synthesized sound via `Beep` or generated PCM only if stable and small.
- High score for the current process session.

## Scope cuts

Cut in this order when behind schedule or over budget:

1. Sound and decorative particles.
2. Secondary weapons and elite variants.
3. Advanced menus and mouse UI.
4. External assets of every kind.

Never cut movement, spawning/pursuit, automatic combat, XP/upgrade choice, death/restart, HUD, or the native EXE release gate.
