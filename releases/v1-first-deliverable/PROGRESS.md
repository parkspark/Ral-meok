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
