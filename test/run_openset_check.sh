#!/usr/bin/env bash
set -uo

# 自动批量测试 open_set 下所有 .pas 文件，记录编译器报错的用例

OPENSET_DIR="$(cd "$(dirname "$0")/open_set" && pwd)"
PASCC_BIN="../build/pascc"
LOG="failed_cases.log"

cd "$(dirname "$0")"

if [[ ! -x $PASCC_BIN ]]; then
  echo "[ERROR] pascc 编译器未找到，请先编译！"
  exit 1
fi

> "$LOG"

for pas in "$OPENSET_DIR"/*.pas; do
  name=$(basename "$pas")
  echo "[INFO] 测试 $name ..."
  "$PASCC_BIN" -i "$pas" > /dev/null 2> err.log
  if [[ $? -ne 0 ]]; then
    echo "$name" | tee -a "$LOG"
    cat err.log >> "$LOG"
    echo "----------------------" >> "$LOG"
  fi
  rm -f err.log
done

echo "\n[完成] 有报错的用例已记录在 $LOG"
