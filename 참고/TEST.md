# Verification Protocol

## Test philosophy

Evidence must match the claim. Compilation proves only compilation; a size check proves only package size. Release success requires build, static/package, launch, and gameplay evidence.

## Gate 1 — Clean release build

From the repository root:

```powershell
Remove-Item -LiteralPath .\dist\Ralphthon.exe -Force -ErrorAction SilentlyContinue
.\build.ps1
if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }
if (-not (Test-Path -LiteralPath .\dist\Ralphthon.exe -PathType Leaf)) { throw 'Release EXE missing' }
```

Pass: the build returns zero and creates a fresh non-empty `dist/Ralphthon.exe`.

## Gate 2 — Package size

Run the exact recursive audit from `PACKAGE.md`.

Pass: all required payload files total no more than **1,474,560 bytes**. Record the exact total, not “about 1.44 MB.”

## Gate 3 — Native dependency audit

Inspect the executable using `dumpbin`, `llvm-objdump`, or `objdump` as described in `PACKAGE.md`.

Pass: it uses Windows system libraries only, or every additional required file is present, measured, and verified. The game does not invoke or embed a browser.

## Gate 4 — Launch smoke test

Launch the exact packaged file:

```powershell
$p = Start-Process -FilePath (Resolve-Path .\dist\Ralphthon.exe) -PassThru
Start-Sleep -Seconds 3
if ($p.HasExited) { throw "Game exited early with code $($p.ExitCode)" }
Stop-Process -Id $p.Id
```

Pass: the process remains alive for three seconds, displays its own native window, and exits cleanly through the normal close action in a manual follow-up. Forced termination is only automation cleanup, not proof of clean exit.

## Gate 5 — Core gameplay scenario

Manually test the packaged executable from a fresh launch:

- [ ] Window appears without an error dialog.
- [ ] WASD and arrow keys move the player; diagonal movement is not faster.
- [ ] Enemies continuously appear and pursue the player.
- [ ] The weapon fires without an attack key and can kill enemies.
- [ ] Kills or drops increase XP and eventually trigger level-up.
- [ ] Level-up pauses combat and presents at least two choices.
- [ ] A choice resumes combat and changes a stat or behavior.
- [ ] At least three distinct upgrade types can be observed across runs/levels.
- [ ] Enemy contact reduces HP, with no single-frame instant drain from one contact.
- [ ] Difficulty visibly increases over time.
- [ ] HUD updates HP, level/XP, survival time, and kills.
- [ ] At zero HP, combat stops and game-over information appears.
- [ ] `R` restarts with HP, timers, entities, XP, level, and kills reset.
- [ ] A continuous play session of at least 60 seconds does not crash or lock.

Pass: every item is checked on the final packaged build.

## Gate 6 — Focused regression checks

Run these after relevant changes:

- Movement: press opposing directions, diagonal directions, and move against all arena edges.
- Targeting: observe behavior with zero enemies, one enemy, and entity-cap pressure.
- Collisions: test projectile hit, enemy death, pickup/XP, player hit, and invulnerability interval.
- Leveling: trigger consecutive level-ups and ensure pause/resume cannot get stuck.
- Restart: die and restart at least twice in one process.
- Capacity: survive through the highest tested spawn pressure without array overflow or severe stall.
- Timing: drag/move the window or lose focus; simulation should not jump catastrophically on return.

## Severity and repair order

- BLOCKER: cannot build, launch, play, restart, or fit within the byte limit; crash or progression dead end.
- MAJOR: required mechanic missing or materially broken; severe input, collision, timing, or performance defect.
- MINOR: visual roughness, balance issue, weak feedback, or harmless text/layout problem.

Repair BLOCKER, then MAJOR, then MINOR. After minute 47, only release-blocking changes are allowed.

## Final release record

Append this evidence to `RALPH_LOG.md` before reporting success:

```text
RELEASE BUILD: PASS/FAIL — command and exit code
PACKAGE SIZE: PASS/FAIL — N / 1,474,560 bytes
DEPENDENCIES: PASS/FAIL — imported DLL summary
LAUNCH: PASS/FAIL — exact packaged EXE tested
GAMEPLAY: PASS/FAIL — checklist completed on final build
KNOWN ISSUES: none, or explicit list with severity
```

Do not use `SUCCESS` if any line is FAIL, missing, or based on a different binary.
