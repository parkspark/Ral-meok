## Loop 1 - DONE
Task: T-01 through T-22 implemented as one compact vertical-slice pass.
Changed: added `src/main.c`, `README.md`, checked `plan/BACKLOG.md`, fixed the PowerShell quoting for `-Wl,--gc-sections` in `PACKAGE.md`.
Build: PASS - `gcc -Os -s -mwindows .\src\*.c -o .\build\game.exe`, exit code 0.
Size: PASS - `build/game.exe` is 26,624 / 1,474,560 bytes, remaining 1,447,936 bytes.
Dependencies: PASS - imported DLLs observed with objdump: GDI32.dll, KERNEL32.dll, USER32.dll, Windows api-ms-win-crt-* UCRT DLLs.
Launch: PASS - final EXE launched, produced a nonzero MainWindowHandle, stayed alive after 3 seconds, and closed via CloseMainWindow with exit code 0.
Gameplay/stability: PASS - same final EXE stayed alive for 120 seconds, covering the 30s and 60s difficulty transitions, then closed normally.
Input smoke: PASS - posted WASD/Arrow movement input and Space to the final EXE window; process stayed alive and closed normally.
Static safety: PASS - fixed arrays for enemies/projectiles, inactive entity checks, zero-vector guard, delta clamp, restart state reset, and GDI backbuffer cleanup.
Known issues: no blocker or major issue known.
Next: optional human feel pass only; no required MVP work remains.

## Loop 2 - Content expansion - DONE

Task: preserve the MVP, then add a connected action-roguelite content loop.

Changed: `src/main.c`, `README.md`, `plan/EXPANSION.md`,
`plan/BACKLOG.md`, `evidence/TEST-REPORT.md`, and versioned releases.

Build: PASS - warning-free C99 review build and exact spec build both succeeded.

Gameplay: PASS - actual EXE verified the shop, level draft, three weapon systems,
elite, 60-second ranged boss, boss kill, relic draft, game over, and Space restart.

Stability: PASS - the same final EXE was alive at 30/60/90/120-second
checkpoints, then closed through its window with exit code 0. A restart at 85
seconds produced a fresh run that continued for more than 30 seconds.

Size: PASS - 33,792 / 1,474,560 bytes, leaving 1,440,768 bytes.

Decision: changed the 60-second extra pressure from every-spawn duplication to
alternating extra spawns and shortened the boss fight so the first relic loop is
reachable during ordinary play.

Next: feature freeze; only an optional human feel/balance pass remains.
