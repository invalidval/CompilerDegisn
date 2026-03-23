#!/usr/bin/env bash
set -euo pipefail

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

if ! "$BIN" --help 2>&1 | grep -q -- "--parse-only"; then
  echo "Current pascc does not support --parse-only."
  echo "Rebuild required: bash scripts/build.sh"
  exit 1
fi

OUT_DIR="$PROJECT_ROOT/test/.parser-test-out"
mkdir -p "$OUT_DIR"

valid_total=0
valid_pass=0
invalid_total=0
invalid_pass=0

echo "[parser] running valid cases..."
for f in "$PROJECT_ROOT"/test/cases/parser_valid/*.pas; do
  valid_total=$((valid_total + 1))
  base=$(basename "$f" .pas)
  stdout_file="$OUT_DIR/${base}.stdout"
  stderr_file="$OUT_DIR/${base}.stderr"

  echo "[parser][valid] $base"
  if "$BIN" -i "$f" --parse-only >"$stdout_file" 2>"$stderr_file"; then
    valid_pass=$((valid_pass + 1))
  else
    echo "VALID FAIL: $f"
    echo "--- stderr ---"
    cat "$stderr_file"
  fi
done

echo "[parser] running invalid cases..."
for f in "$PROJECT_ROOT"/test/cases/parser_invalid/*.pas; do
  invalid_total=$((invalid_total + 1))
  base=$(basename "$f" .pas)
  stdout_file="$OUT_DIR/${base}.stdout"
  stderr_file="$OUT_DIR/${base}.stderr"

  echo "[parser][invalid] $base"
  set +e
  if command -v timeout >/dev/null 2>&1; then
    timeout 8s "$BIN" -i "$f" --parse-only >"$stdout_file" 2>"$stderr_file"
    code=$?
    timed_out=0
    if [[ $code -eq 124 ]]; then
      timed_out=1
    fi
  else
    "$BIN" -i "$f" --parse-only >"$stdout_file" 2>"$stderr_file"
    code=$?
    timed_out=0
  fi
  set -e

  if [[ $timed_out -eq 1 ]]; then
    echo "INVALID FAIL: $f"
    echo "exit_code=$code (timeout)"
    echo "--- stderr ---"
    cat "$stderr_file"
  elif [[ $code -ne 0 ]] && grep -q "Parse error\|Parsing failed" "$stderr_file"; then
    invalid_pass=$((invalid_pass + 1))
  else
    echo "INVALID FAIL: $f"
    echo "exit_code=$code"
    echo "--- stderr ---"
    cat "$stderr_file"
  fi
done

echo "[parser] summary"
echo "valid:   ${valid_pass}/${valid_total}"
echo "invalid: ${invalid_pass}/${invalid_total}"

if [[ $valid_pass -eq $valid_total ]] && [[ $invalid_pass -eq $invalid_total ]]; then
  echo "Parser test passed."
  exit 0
fi

echo "Parser test failed."
exit 1
