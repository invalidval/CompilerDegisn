#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
echo "Verifying shell scripts under: $ROOT_DIR"

shopt -s globstar 2>/dev/null || true

MISSING_SHEBANG=()
NOT_EXECUTABLE=()

for f in "$ROOT_DIR"/**/*.sh; do
  [ -f "$f" ] || continue
  # Check executable bit
  if [ ! -x "$f" ]; then
    NOT_EXECUTABLE+=("$f")
  fi
  # Check shebang
  if ! head -n 1 "$f" | grep -q '^#!'; then
    MISSING_SHEBANG+=("$f")
  fi
done

if [ ${#MISSING_SHEBANG[@]} -gt 0 ]; then
  echo "ERROR: The following scripts are missing a shebang (#!/...):"
  for s in "${MISSING_SHEBANG[@]}"; do echo "  $s"; done
  exit 1
fi

if [ ${#NOT_EXECUTABLE[@]} -gt 0 ]; then
  echo "WARNING: The following scripts are not executable (chmod +x):"
  for s in "${NOT_EXECUTABLE[@]}"; do echo "  $s"; done
fi

echo "All checked shell scripts appear executable with proper shebangs where required."
