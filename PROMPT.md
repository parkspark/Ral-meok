# PROMPT.md — 개별 Ralph Loop 체크리스트

이 파일은 과거 `loop.sh`의 반복 `codex exec` 입력문이 아니라, 활성 Goal 내부에서 각 루프의 일관성을 확인하기 위한 체크리스트다.

1. `AGENTS.md`, `PLAN.md`, `TEST.md`, `PACKAGE.md`의 현재 규칙을 따른다.
2. `PROGRESS.md`와 `plan/BACKLOG.md`를 읽고 가장 위의 실행 가능한 미완료 작업 하나를 선택한다.
3. 사용자에게 보이거나 테스트로 관찰 가능한 완료 조건을 먼저 정한다.
4. 해당 작업만 구현한다. 실패 수정은 같은 작업으로 본다.
5. 관련 집중 테스트와 release 빌드를 수행한다.
6. `build/game.exe`의 정확한 바이트 수를 확인한다.
7. 검증 성공 시에만 BACKLOG를 체크하고 PROGRESS에 증거를 기록한다.
8. 실패 시 오류, 시도, 다음 해소책을 기록하고 새 기능 대신 복구를 계속한다.
9. 현재 시간 게이트를 확인한다. 47분 이전이면 다음 루프를 계속하고, 이후면 새 기능을 금지한다.

한 루프의 출력 형식:

```text
LOOP: N / 경과 MM:SS
TASK: T-XX
DONE: 관찰 가능한 변화
TEST: 실행한 명령과 시나리오
RESULT: PASS 또는 FAIL + 근거
SIZE: N / 1,474,560 bytes
NEXT: 다음 작업 또는 복구 단계
```
