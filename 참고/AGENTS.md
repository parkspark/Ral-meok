# Ralphthon Codex Harness

## Mission

When the user starts a Codex `goal`, build and continuously improve a Windows-native survivor game for 50 minutes. The final submission must run directly on Windows and the complete distributable must be at most **1,474,560 bytes**.

This file governs the implementation loop. Read `PLAN.md`, `PACKAGE.md`, and `TEST.md` before changing code.

## Non-negotiable constraints

- Agent: Codex running directly on Windows.
- Workflow: Ralph loop—inspect, choose one objective, implement, test, record, repeat.
- Genre: Vampire Survivors-like arena survival.
- Delivery: native standalone Windows `.exe`; browser output is forbidden.
- Size: every file required to play, including runtimes and data, counts toward the exact 1,474,560-byte ceiling.
- Default stack: C17 + Win32 API + GDI, linked as a native release executable.
- Dependencies: Windows system DLLs only. No engine, redistributable, installer, external runtime, downloaded asset, or network dependency.
- Assets: procedural drawing and generated audio only; do not ship image, font, or audio files unless `PACKAGE.md` proves the budget still passes.
- The checked distributable is `dist/Ralphthon.exe`. Debug files and source files are not part of `dist/`.

## Required gameplay loop

The minimum shippable build must have all of the following:

1. Immediate start and WASD/arrow-key movement.
2. Continuous enemy spawning and player pursuit.
3. At least one automatic weapon with aiming, cooldown, hits, and enemy death.
4. Player HP, contact damage, brief invulnerability, death, and restart.
5. XP gain, levels, and a paused choice of at least two upgrades.
6. At least three upgrades that measurably change play.
7. HUD for HP, level/XP, time, and kills.
8. Difficulty escalation over time.
9. A complete playable loop lasting at least one minute.

## Working rules

- Work only inside this repository. Preserve unrelated user changes.
- Prefer a tiny number of `.c`/`.h` files and no third-party packages.
- Maintain a runnable build after every loop. Fix regressions before adding features.
- Do not spend time on architecture that cannot be seen or tested in the final game.
- Use deterministic or seeded randomness where it helps reproduce bugs.
- Keep simulation updates independent of rendering where practical.
- Treat compiler warnings, crashes, broken input, and size overflow as blockers.
- Never claim a feature works without running the relevant check in `TEST.md`.
- Never claim the package passes without measuring every file under `dist/`.

## Ralph loop

Repeat until the 50-minute deadline. Each loop should be small enough to finish and verify in roughly 2–5 minutes.

1. Inspect the repository, current build, remaining time, latest test result, and package size.
2. Pick the highest-priority incomplete item from `PLAN.md`.
3. State one observable completion condition.
4. Make the smallest coherent implementation that satisfies it.
5. Build and run the narrow checks from `TEST.md`.
6. If a check fails, repair it in the same loop; do not layer new work on failure.
7. Append a compact entry to `RALPH_LOG.md` using the format below.
8. Re-rank the next item based on risk and remaining time.

```text
[MM:SS] LOOP N
GOAL: one observable outcome
CHANGED: files or behavior
TEST: command/manual check
RESULT: PASS/FAIL and evidence
SIZE: dist total in bytes
NEXT: highest-priority remaining item
```

Create `RALPH_LOG.md` when implementation begins; it is a work log, not a required submission file.

## Time gates

- 00–05: toolchain discovery, skeleton window, build script, first launch.
- 05–15: movement, fixed-step loop, enemies, spawning, pursuit.
- 15–25: automatic weapon, collisions, damage, deaths, XP.
- 25–35: leveling, upgrade choice, three meaningful upgrades.
- 35–42: HUD, game over/restart, escalating difficulty, game feel.
- 42–47: full playtest, blocker/major fixes, size optimization.
- 47–50: feature freeze; clean build, package audit, launch smoke test.

At each gate, cut polish before core loop reliability. After minute 47, add no feature unless it fixes a release blocker.

## Priority order

1. Compiles and launches.
2. Full gameplay loop is connected.
3. No crash or progression blocker.
4. Package is within 1,474,560 bytes.
5. Controls and feedback are readable.
6. Balance and visual/audio polish.

## Definition of done

Completion requires evidence for every release gate in `TEST.md`, a clean `dist/` containing only the playable payload, and a measured total size no greater than 1,474,560 bytes. If any gate is unverified, report partial status instead of success.
