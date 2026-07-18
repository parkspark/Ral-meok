# TEST.md — 검증 프로토콜

## 원칙

빌드 성공은 실행 성공이 아니고, 프로세스 생존은 게임플레이 성공이 아니다. 각 주장은 해당 수준의 증거로 검증하며, 최종 성공은 같은 최종 `build/game.exe`에 대한 모든 게이트가 필요하다.

## Gate 1 — 깨끗한 release 빌드

`PACKAGE.md`의 표준 release 명령을 실행한다.

PASS 조건:

- GCC exit code 0
- 새 `build/game.exe` 존재 및 크기 > 0
- 컴파일 오류 없음

경고가 있으면 관련 코드의 안전성을 검토하고 `PROGRESS.md`에 남긴다.

## Gate 2 — 정적 안전성 리뷰

- [ ] Enemy/Projectile 인덱스가 배열 경계를 넘지 않는다.
- [ ] 거리 0인 벡터를 나누기 전에 guard한다.
- [ ] 프레임 delta가 지나치게 클 때 clamp한다.
- [ ] restart가 모든 gameplay 상태와 키 상태를 초기화한다.
- [ ] resize/destroy에서 선택했던 GDI 객체를 복원하고 삭제한다.
- [ ] inactive entity를 update/collision/render에서 건너뛴다.

## Gate 3 — 네이티브 실행 smoke test

```powershell
$p = Start-Process -FilePath (Resolve-Path .\build\game.exe) -PassThru
Start-Sleep -Seconds 3
if ($p.HasExited) { throw "game exited early: $($p.ExitCode)" }
"PID=$($p.Id) alive after 3 seconds"
```

PASS 조건: 오류 대화상자 없이 자체 Windows 창이 보이고 프로세스가 3초 뒤에도 살아 있다. 확인 후 창의 닫기 버튼으로 종료하고 프로세스가 정상 종료되는지도 확인한다. 자동화가 강제 종료한 결과만으로 정상 종료를 주장하지 않는다.

## Gate 4 — 기능별 집중 검증

관련 작업 직후 해당 행을 확인한다.

| 영역 | 시나리오 | PASS 조건 |
|---|---|---|
| 입력 | WASD, 방향키, 대각선, 반대키 동시 입력 | 이동 가능, 대각선 과속 없음, 화면 밖 이탈 없음 |
| 스폰 | 시작 후 60초 관찰 | 화면 가장자리에서 지속 생성, 풀 초과/멈춤 없음 |
| 추적 | 여러 방향에서 적 접근 | 각 적이 플레이어 쪽으로 움직이고 0거리 오류 없음 |
| 자동 공격 | 적 0/1/다수 상태 | 적 없을 때 안전, 가장 가까운 적 방향 발사 |
| 투사체 | 명중/빗나감/수명 종료 | 피해 적용, 사망 회수, 화면 밖 투사체 회수 |
| 접촉 피해 | 한 적과 연속 접촉 | HP 감소 후 약 0.5초 무적, 즉사성 프레임 중복 없음 |
| 강화 | 킬 임계값 통과 | 공격력/속도 변화가 실제 수치와 플레이에 반영 |
| 난이도 | 30초와 60초 경계 통과 | 세 단계의 스폰/속도 변화가 중복 없이 적용 |
| 게임오버 | HP 0 | 전투 정지, 생존시간/재시작 안내 표시 |
| 재시작 | Space로 두 번 반복 | HP, 킬, 시간, 풀, 강화, 입력 모두 초기화 |

## Gate 5 — 최종 2분 플레이 시나리오

최종 후보 EXE를 새로 실행해 다음 순서로 확인한다.

1. 창과 HUD가 보이는지 확인한다.
2. 모든 이동 방향과 경계를 확인한다.
3. 적 스폰, 추적, 자동 발사, 적 사망을 확인한다.
4. 킬 임계 강화를 최소 한 번 확인한다.
5. 30초와 60초 난이도 전환을 통과한다.
6. 접촉 피해로 게임오버까지 진행한다.
7. Space로 재시작하고 30초 이상 다시 플레이한다.

PASS 조건: 총 2분 이상 크래시/멈춤 없이 핵심 루프가 이어지고, HUD와 피드백이 현재 상태를 정확히 보여준다.

## Gate 6 — 크기와 의존성

`PACKAGE.md`의 정확한 크기 및 DLL 감사를 수행한다.

PASS 조건: 전체 payload ≤ 1,474,560 bytes이고 Windows 기본 시스템 DLL 외 의존성이 없다.

## 결함 등급

- BLOCKER: 빌드/실행/진행/재시작 불가, 크래시, payload 초과, 필수 런타임 누락
- MAJOR: 명세 핵심 기능이 없거나 심각하게 오동작, 배열/타이밍 위험
- MINOR: 가독성, 피드백, 밸런스, 장식 문제

항상 BLOCKER → MAJOR → MINOR 순으로 수정한다.

## 최종 릴리스 레코드

```text
BUILD: PASS/FAIL — 명령, exit code
STATIC SAFETY: PASS/FAIL — 확인 결과
LAUNCH: PASS/FAIL — 최종 EXE와 창 확인
GAMEPLAY 2 MIN: PASS/FAIL — 핵심 시나리오
PACKAGE: PASS/FAIL — N / 1,474,560 bytes
DEPENDENCIES: PASS/FAIL — DLL 목록
KNOWN ISSUES: 등급과 재현법 또는 없음
```

하나라도 FAIL/미검증이면 SUCCESS로 종료하지 않는다.
