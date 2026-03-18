#!/usr/bin/env bash
set -euo pipefail

if [[ -x ./build/pascc ]]; then
  ./build/pascc --help
elif [[ -x ./pascc ]]; then
  ./pascc --help
else
  echo "pascc not found. Run scripts/build.sh first."
  exit 1
fi

echo "Smoke test passed."
