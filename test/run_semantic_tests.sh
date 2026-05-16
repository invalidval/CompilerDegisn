#!/usr/bin/env bash
set -euo

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN=""
if [[ -x "$PROJECT_ROOT/build/pascc" ]]; then
  BIN="$PROJECT_ROOT/build/pascc"
elif [[ -x "$PROJECT_ROOT/build/pascc.exe" ]]; then
  BIN="$PROJECT_ROOT/build/pascc.exe"
elif [[ -x "$PROJECT_ROOT/pascc" ]]; then
  BIN="$PROJECT_ROOT/pascc"
elif [[ -x "$PROJECT_ROOT/pascc.exe" ]]; then
  BIN="$PROJECT_ROOT/pascc.exe"
else
  echo "pascc not found. Run scripts/build.sh first."
  exit 1
fi

if ! "$BIN" --help 2>&1 | grep -q -- "--semantic"; then
  echo "Current pascc does not support --semantic."
  echo "Rebuild required: bash scripts/build.sh"
  exit 1
fi

OUT_DIR="$PROJECT_ROOT/test/.semantic-test-out"
mkdir -p "$OUT_DIR"

valid_total=0
valid_pass=0
invalid_total=0
invalid_pass=0

echo "========================================="
echo "[semantic] running valid cases..."
echo "========================================="
for f in "$PROJECT_ROOT"/test/cases/semantic_valid/*.pas; do
  valid_total=$((valid_total + 1))
  base=$(basename "$f" .pas)
  stdout_file="$OUT_DIR/${base}.stdout"
  stderr_file="$OUT_DIR/${base}.stderr"

  echo "[semantic][valid] $base"
  if "$BIN" -i "$f" --semantic >"$stdout_file" 2>"$stderr_file"; then
    valid_pass=$((valid_pass + 1))
  else
    echo "  VALID FAIL: $f"
    echo "  --- stderr ---"
    cat "$stderr_file"
    echo ""
  fi
done

echo ""
echo "========================================="
echo "[semantic] running invalid cases..."
echo "========================================="
for f in "$PROJECT_ROOT"/test/cases/semantic_invalid/*.pas; do
  invalid_total=$((invalid_total + 1))
  base=$(basename "$f" .pas)
  stdout_file="$OUT_DIR/${base}.stdout"
  stderr_file="$OUT_DIR/${base}.stderr"

  echo "[semantic][invalid] $base"
  set +e
  if command -v timeout >/dev/null 2>&1; then
    timeout 8s "$BIN" -i "$f" --semantic >"$stdout_file" 2>"$stderr_file"
    code=$?
    timed_out=0
    if [[ $code -eq 124 ]]; then
      timed_out=1
    fi
  else
    "$BIN" -i "$f" --semantic >"$stdout_file" 2>"$stderr_file"
    code=$?
    timed_out=0
  fi
  set -e

  if [[ $timed_out -eq 1 ]]; then
    echo "  INVALID FAIL: $f"
    echo "  exit_code=$code (timeout)"
    echo "  --- stderr ---"
    cat "$stderr_file"
    echo ""
  elif [[ $code -ne 0 ]]; then
    invalid_pass=$((invalid_pass + 1))
  else
    echo "  INVALID FAIL: $f"
    echo "  exit_code=$code (expected non-zero)"
    echo "  --- stderr ---"
    cat "$stderr_file"
    echo ""
  fi
done

echo ""
echo "========================================="
echo "[semantic] summary"
echo "========================================="
echo "valid:   ${valid_pass}/${valid_total}"
echo "invalid: ${invalid_pass}/${invalid_total}"

if [[ $valid_pass -eq $valid_total ]] && [[ $invalid_pass -eq $invalid_total ]]; then
  echo ""
  echo "All semantic tests passed."
  exit 0
fi

echo ""
echo "Semantic tests failed."
exit 1
