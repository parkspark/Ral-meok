# Content Expansion Test Report

## Release candidate

- Command: `gcc -Os -s -mwindows .\src\*.c -o .\build\game.exe`
- Build: PASS, exit code 0
- Strict review build: PASS with `-std=c99 -Wall -Wextra`, no warnings
- Size: PASS, 33,792 / 1,474,560 bytes
- Remaining: 1,440,768 bytes
- SHA-256: `B24FDA6B1BF25EA4D9F7B1DE19B0DB7DAE05C23E5F57E1B39852B6B88C5A18D0`

## Runtime gates

- Native launch: PASS; nonzero window handle, alive after smoke delay.
- Shop: PASS; `B` opened the paused Night Market and the window closed normally.
- Level draft: PASS; XP opened a paused three-card choice with a guaranteed weapon.
- Input: PASS; automated WASD movement and numeric choices were accepted.
- Boss combat: PASS; 60-second boss, boss HP bar, and aimed hostile shots observed.
- Boss reward: PASS; HUD changed to `BOSSES 1 RELICS 1` after the kill and choice.
- Two-minute stability: PASS; alive at 30, 60, 90, and 120-second checkpoints.
- Restart scenario: PASS; Space at 85 seconds produced a fresh run shown at
  34 seconds with reset HP, level, XP, boss, and relic state.
- Shutdown: PASS; `CloseMainWindow()` returned true and process exited with code 0.

## Static safety

- Enemy, player-shot, enemy-shot, and effect pools use fixed arrays.
- Every entity loop is bounded and inactive slots are skipped.
- Zero-distance normalization is guarded and frame delta is clamped.
- Menu states pause combat and use edge-triggered input.
- Restart clears all pools, keys, timers, progression, shop, boss, and relic state.
- Selected GDI objects are restored before bitmap/DC deletion.

## Dependencies

`objdump -p` reports only Windows system libraries: GDI32, KERNEL32, USER32,
and Windows UCRT `api-ms-win-crt-*` components. No external game library,
asset, or sidecar runtime file is required.

## Visual evidence

- `shop-smoke.png`: Night Market layout and paused combat.
- `level-choice.png`: level-up three-choice skill draft.
- `boss-proof-72.png`: live boss, HP bar, projectiles, and multi-skill build.
- `boss-proof-88.png`: post-boss HUD with one boss and one relic.
- `final-restart-120s.png`: restarted run alive for more than 30 seconds.

Known blocker/major issues: none found. Difficulty is intentionally lethal;
the expected two-minute scenario includes death, restart, and continued play.
