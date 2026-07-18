# PACKAGE.md — 1,474,560-byte 배포 계약

## 하드 리밋

실행에 필요한 전체 payload의 합은 **1,474,560 bytes 이하**여야 한다. “약 1.44MB”나 디스크의 반올림 표시는 증거가 아니다.

기본 배포물은 다음 한 파일이다.

```text
build/game.exe
```

소스, 문서, PDB, 오브젝트, 로그는 심사용 저장소에는 남아도 되지만 실행 payload에는 포함하지 않는다. 별도 DLL이나 데이터가 필요해지면 모두 합산해야 하며, 원칙적으로 그런 의존성을 만들지 않는다.

## 표준 release 빌드

```powershell
New-Item -ItemType Directory -Force .\build | Out-Null
gcc -std=c99 -Os -s -ffunction-sections -fdata-sections -mwindows .\src\*.c '-Wl,--gc-sections' -lgdi32 -luser32 -lkernel32 -o .\build\game.exe
if ($LASTEXITCODE -ne 0) { throw "release build failed: $LASTEXITCODE" }
```

환경의 GCC가 일부 옵션을 지원하지 않으면 `specs/game-spec.md`의 최소 명령으로 되돌린다. 명령을 바꾼 이유와 최종 명령을 `PROGRESS.md` 및 `README.md`에 기록한다.

## 정확한 크기 게이트

```powershell
$limit = 1474560
$payload = @('.\build\game.exe')
$missing = $payload | Where-Object { -not (Test-Path -LiteralPath $_ -PathType Leaf) }
if ($missing) { throw "missing payload: $missing" }
$total = ($payload | ForEach-Object { (Get-Item -LiteralPath $_).Length } | Measure-Object -Sum).Sum
"TOTAL=$total LIMIT=$limit REMAINING=$($limit-$total)"
if ($total -gt $limit) { throw "payload exceeds limit by $($total-$limit) bytes" }
```

## 의존성 게이트

첫 번째로 사용 가능한 도구를 사용한다.

```powershell
objdump -p .\build\game.exe | Select-String 'DLL Name'
dumpbin /dependents .\build\game.exe
llvm-objdump -p .\build\game.exe | Select-String 'DLL Name'
```

Windows 기본 시스템 DLL 외의 런타임 DLL이 나오면 BLOCKER다. DLL을 동봉해 크기만 맞추는 것보다 해당 의존성을 제거한다.

## 초과 시 감량 순서

1. stale EXE, PDB, map, 로그가 payload 계산에 섞였는지 확인한다.
2. `-Os -s`, section garbage collection이 실제 release 명령에 적용됐는지 확인한다.
3. 디버그 문자열, `printf`/stdio, 사용하지 않는 분기와 테이블을 제거한다.
4. 장식 효과와 중복 텍스트를 단순화한다.
5. 외부 데이터가 생겼다면 절차적 GDI 표현으로 치환한다.

필요한 DLL이나 데이터를 숨기거나 제외해서 통과한 것처럼 기록하지 않는다.

## 릴리스 증거

최종 `PROGRESS.md`에 컴파일러 버전, 정확한 명령, EXE 바이트 수, 남은 바이트, imported DLL 목록, 동일 EXE의 실행 결과를 남긴다.
