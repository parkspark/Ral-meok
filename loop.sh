#!/usr/bin/env bash
# 사용법: ./loop.sh
# 50분(3000초) 동안 시간 기준으로 멈춤없이 도는 랄프 루프.
# 반복 횟수가 아니라 '경과 시간'이 종료 조건이다.
set -uo pipefail

DURATION_SEC=$((50 * 60))   # 50분
SIZE_LIMIT=1474560          # 1.44MB (1440*1024)
START=$(date +%s)
FAIL_STREAK=0
ITER=0

echo "===== 랄프 루프 시작 (제한시간 50분) ====="

while true; do
  NOW=$(date +%s)
  ELAPSED=$((NOW - START))
  REMAIN=$((DURATION_SEC - ELAPSED))

  if [ "$REMAIN" -le 0 ]; then
    echo "⏱ 50분 경과. 루프 종료."
    break
  fi

  # 백로그 전체 완료 시 조기 종료
  if ! grep -q "^- \[ \]" plan/BACKLOG.md; then
    echo "✅ 백로그 전체 완료. 시간이 남았지만 종료합니다."
    break
  fi

  ITER=$((ITER + 1))
  echo "----- 회차 #$ITER (남은 시간: ${REMAIN}초) -----"

  codex exec \
    --sandbox workspace-write \
    --ask-for-approval never \
    "$(cat PROMPT.md)"

  # 진전 확인: 최근 90초 내 커밋이 있었는지
  if git log --oneline -1 --since="90 seconds ago" | grep -q .; then
    FAIL_STREAK=0
  else
    FAIL_STREAK=$((FAIL_STREAK + 1))
    echo "⚠️ 이번 회차 커밋 없음 (연속 ${FAIL_STREAK}회)"
  fi

  # exe 크기 체크 — 경고만 하고 루프는 계속 진행 (여기서 멈추지 않음)
  if [ -f build/game.exe ]; then
    SIZE=$(stat -c%s build/game.exe 2>/dev/null || echo 0)
    if [ "$SIZE" -gt "$SIZE_LIMIT" ]; then
      echo "⚠️ game.exe ${SIZE} bytes — 1.44MB 초과 (경고, 계속 진행)"
    else
      echo "✅ game.exe ${SIZE} bytes"
    fi
  fi

  # 서킷 브레이커: 3회 연속 무진전이면 사람 개입 필요 → 종료
  if [ "$FAIL_STREAK" -ge 3 ]; then
    echo "🛑 서킷 브레이커 작동 (연속 3회 무진전). PROGRESS.md의 BLOCKED 확인 후 사람이 개입할 것."
    break
  fi

  sleep 3
done

echo "===== 루프 종료: 총 ${ITER}회 실행 ====="
if [ -f build/game.exe ]; then
  FINAL_SIZE=$(stat -c%s build/game.exe 2>/dev/null || echo "?")
  echo "최종 game.exe 크기: ${FINAL_SIZE} bytes (제한: ${SIZE_LIMIT} bytes)"
fi
