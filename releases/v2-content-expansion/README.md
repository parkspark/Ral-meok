# Ralphthon Survivor: Relic Run

외부 에셋 없이 Win32 API와 GDI만으로 만든 단일 EXE 액션 로그라이트입니다.
이동만 직접 조작하고, 자동 공격과 런 중 선택으로 빌드를 완성합니다.

## 빌드

```powershell
New-Item -ItemType Directory -Force .\build | Out-Null
gcc -Os -s -mwindows .\src\*.c -o .\build\game.exe
```

GCC가 현재 창의 PATH에 아직 반영되지 않았다면:

```powershell
$env:Path='C:\Users\king2\AppData\Local\Programs\WinLibs\mingw64\bin;' + $env:Path
```

## 실행과 조작

```powershell
.\build\game.exe
```

- `WASD` 또는 방향키: 이동
- `B`: 전투를 일시정지하고 인런 상점 열기/닫기
- 선택 화면과 상점의 `1`, `2`, `3`: 항목 선택 또는 구매
- 상점의 `4`: 전투 복귀
- 게임오버의 `Space`: 새 랜덤 런 시작
- 전투 중 `Esc`: 종료

## 콘텐츠

- 가장 가까운 적을 향하는 자동 Arc Bolt
- 레벨업마다 무기 1종을 반드시 포함하는 3지선다
- Arc Bolt, Orbit Blades, Nova Pulse와 공격력·이동·체력 강화
- 일반 적, 고보상 엘리트, 60초 주기의 원거리 보스
- 보스 처치 후 6종 유물 중 3지선다
- 처치 골드를 회복·공격력·최대 HP에 쓰는 Night Market
- XP/HP/골드/스킬/보스/유물 HUD, 보스 경고와 전투 피드백
- 사망 시 런 빌드를 초기화하는 로그라이트 재시작

## 최종 검증 빌드

- 파일: `build/game.exe`
- 크기: **33,792 / 1,474,560 bytes**
- 남은 용량: **1,440,768 bytes**
- SHA-256: `B24FDA6B1BF25EA4D9F7B1DE19B0DB7DAE05C23E5F57E1B39852B6B88C5A18D0`
- 외부 런타임/에셋: 없음
- 검증: 경고 없는 C99 빌드, 실행/입력/상점/레벨업/보스/유물,
  2분 프로세스 안정성, 사망 후 재시작 30초 이상, 정상 창 종료

보존된 MVP는 `releases/v1-first-deliverable`, 콘텐츠 확장판은
`releases/v2-content-expansion`에 있습니다. 자세한 증거는
`evidence/TEST-REPORT.md`를 참고합니다.
