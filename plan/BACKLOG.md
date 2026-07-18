# BACKLOG.md - Ralphthon Goal Execution Order

Each item must be completed with build or runtime evidence before it is checked.

## P0 - Playable Vertical Slice

- [x] T-01 Toolchain/project boot: GCC verified, build folder, WinMain, 800x600 window, closable message loop.
- [x] T-02 Stable frame loop: QueryPerformanceCounter timing, delta clamp, update/render split, GDI backbuffer create/destroy.
- [x] T-03 Player control: player state, WASD/Arrow movement, diagonal normalization, screen bounds, visible render.
- [x] T-04 Enemy pool and spawn: fixed enemy array, edge spawn, player chasing, visible render.
- [x] T-05 Contact survival: circular collision, HP 100, contact damage, 0.5s invulnerability, HP gauge and hit feedback.
- [x] T-06 Projectile pool: fixed projectile array, nearest-enemy targeting, cooldown auto-fire, movement, lifetime, cleanup.
- [x] T-07 Kill loop: projectile collision, enemy HP/damage, kill count, hit/kill feedback.
- [x] T-08 End loop: HP 0 game over, combat pause, survival/kills display, Space restart.

## P1 - Spec Completion

- [x] T-09 Three difficulty stages: 0-30s, 30-60s, 60s+ spawn interval and enemy speed changes.
- [x] T-10 Kill-based upgrades: thresholds increase damage and fire rate with an upper cap.
- [x] T-11 HUD: HP, time, kills, power state shown without overlap.
- [x] T-12 Safety review: array bounds, zero-vector guard, delta clamp, inactive entity skip, GDI cleanup, repeat restart path.
- [x] T-13 Two-minute integration run: final EXE stayed alive through 120s including 30s/60s phase changes.

## P2 - Low-Cost Polish

- [x] T-14 Combat readability: shot flashes, hit/kill flashes, player hit flash.
- [x] T-15 Screen finish: grid background, HP gauge, footer control hint, centered game over panel.
- [x] T-16 Risk-light feel pass: small screen shake and stage pressure through spawn/speed scaling.
- [x] T-17 Balance pass: enemy/projectile/player values tuned for immediate readable combat.
- [x] T-18 Game feel pass: nearest-target auto-fire and power growth feedback.

## P3 - Release

- [x] T-19 Size/DLL audit: payload and imported DLLs recorded.
- [x] T-20 Final smoke: same final EXE launched, produced a window, survived 3s, and closed normally.
- [x] T-21 README: build, run, controls, features, and size contract documented.
- [x] T-22 Final evidence: release record written to PROGRESS.md.

## Content expansion - completed

- [x] X-01 Preserve the verified MVP as an immutable first deliverable.
- [x] X-02 Add XP, levels, coins, and randomized three-choice run progression.
- [x] X-03 Add Arc Bolt, Orbit Blades, Nova Pulse, and passive skill upgrades.
- [x] X-04 Add distinct elites and a timed ranged boss with clear silhouettes.
- [x] X-05 Add boss relic drafts with six stackable relic effects.
- [x] X-06 Add a paused in-run shop with healing and permanent run upgrades.
- [x] X-07 Add boss warnings, enemy HP bars, expanded HUD, and run summary.
- [x] X-08 Rebalance the 60-second pressure band and boss encounter.
- [x] X-09 Verify shop, draft, boss, relic, restart, and two-minute stability.
- [x] X-10 Freeze the verified expansion as the second deliverable.
