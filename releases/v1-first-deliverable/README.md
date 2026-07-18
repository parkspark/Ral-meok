# Ralphthon Survivor

Tiny Windows-native Vampire Survivors-style MVP for the Ralphthon Codex Goal run.

## Build

```powershell
New-Item -ItemType Directory -Force .\build | Out-Null
gcc -std=c99 -Os -s -ffunction-sections -fdata-sections -mwindows .\src\*.c '-Wl,--gc-sections' -lgdi32 -luser32 -lkernel32 -o .\build\game.exe
```

Fallback spec command:

```powershell
gcc -Os -s -mwindows .\src\*.c -o .\build\game.exe
```

## Run

```powershell
.\build\game.exe
```

## Controls

- WASD or Arrow keys: move
- Space: restart after game over
- Esc: quit

## Implemented Features

- 800x600 Win32/GDI standalone EXE
- player movement with diagonal normalization and screen bounds
- edge enemy spawning and player-chasing enemies
- automatic fire at the nearest enemy
- projectile lifetime, collision, kills, hit feedback
- player HP, contact damage, short invulnerability
- kill-threshold attack damage and fire-rate upgrades
- 0-30s, 30-60s, 60s+ difficulty phases
- game over and Space restart
- HUD with HP, time, kills, and power

## Size Contract

Final payload is `build/game.exe` only and must be at most 1,474,560 bytes.

Current verified build:

- command: `gcc -Os -s -mwindows .\src\*.c -o .\build\game.exe`
- size: 26,624 bytes
- limit remaining: 1,447,936 bytes
- smoke: launched final EXE, visible window handle, alive after 3 seconds, closed normally
- stability: same final EXE stayed alive for 120 seconds and closed normally
- input smoke: WASD/Arrow movement messages and Space were sent to the final EXE window; it stayed alive and closed normally
