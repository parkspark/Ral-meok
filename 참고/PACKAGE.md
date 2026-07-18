# Packaging and Size Contract

## Hard limit

The complete playable distribution must be **less than or equal to 1,474,560 bytes**. This is the binary interpretation of a 1.44 MB floppy and is the authoritative contest ceiling.

Everything required to run counts: executable, data, DLLs, runtime, configuration, assets, and launch scripts. Source, debug symbols, build intermediates, and documentation do not belong in `dist/` unless they are required for play.

## Release payload

Target payload:

```text
dist/
  Ralphthon.exe
```

The game must run by launching that file on Windows without a browser, installer, engine installation, network, environment variable, or copied runtime.

## Allowed dependencies

Only DLLs that ship as part of supported Windows installations may be dynamically imported, such as `kernel32.dll`, `user32.dll`, and `gdi32.dll`. Avoid toolchain-specific runtime DLLs.

- MSVC: prefer `/O1 /GL /Gy /Gw /DNDEBUG`, link `/LTCG /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS`; use the static CRT only if CRT functionality is needed and the measured result remains small.
- MinGW/LLVM: prefer `-Os -ffunction-sections -fdata-sections` with linker garbage collection and GUI subsystem; avoid dependencies on `libgcc_s`, `libstdc++`, or other non-system DLLs.
- Do not use UPX unless contest rules explicitly permit executable compression and the compressed EXE is launch-tested on the target environment.

Exact compiler flags may be adapted to the installed toolchain. A successful clean build and dependency audit are more important than copying flags blindly.

## PowerShell size audit

Run this from the repository root after every release build:

```powershell
$limit = 1474560
$files = Get-ChildItem -LiteralPath .\dist -File -Recurse
$total = ($files | Measure-Object -Property Length -Sum).Sum
if ($null -eq $total) { $total = 0 }
$files | Select-Object FullName, Length
"TOTAL=$total LIMIT=$limit REMAINING=$($limit - $total)"
if ($files.Count -eq 0) { throw 'dist is empty' }
if ($total -gt $limit) { throw "Package exceeds limit by $($total - $limit) bytes" }
```

Success requires exit code 0, at least one payload file, and `TOTAL` no greater than `1474560`.

## Dependency audit

Use the first locally available option:

```powershell
dumpbin /dependents .\dist\Ralphthon.exe
llvm-objdump -p .\dist\Ralphthon.exe
objdump -p .\dist\Ralphthon.exe
```

Inspect imported DLL names. If a non-Windows runtime DLL is required, it must be placed in `dist/`, included in the total, and tested on a clean Windows environment; preferably remove that dependency.

## Size response ladder

If over budget:

1. Confirm `dist/` contains no PDB, map, object, source, screenshot, or stale executable.
2. Enable release optimization, dead-code elimination, and identical-code folding.
3. Remove unused strings, debug logging, assets, and dormant code paths.
4. Replace external assets with procedural GDI drawing.
5. Reduce lookup tables and duplicated content.
6. Rebuild cleanly and rerun both size and launch checks.

Never delete required runtime files merely to make the measurement pass.

## Packaging evidence

Record in `RALPH_LOG.md`:

- compiler and exact release command;
- every file and byte count under `dist/`;
- total bytes and remaining headroom;
- imported dependency result;
- launch smoke-test result for the exact packaged executable.
