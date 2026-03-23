#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if [[ -x "$PROJECT_ROOT/build/pascc" ]]; then
  "$PROJECT_ROOT/build/pascc" --help
elif [[ -x "$PROJECT_ROOT/build/pascc.exe" ]]; then
  "$PROJECT_ROOT/build/pascc.exe" --help
elif [[ -x "$PROJECT_ROOT/pascc" ]]; then
  "$PROJECT_ROOT/pascc" --help
elif [[ -x "$PROJECT_ROOT/pascc.exe" ]]; then
  "$PROJECT_ROOT/pascc.exe" --help
else
  echo "pascc not found. Run scripts/build.sh first."
  exit 1
fi

echo "Smoke test passed."
