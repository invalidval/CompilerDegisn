#!/usr/bin/env bash
set -euo pipefail

if command -v cmake >/dev/null 2>&1; then
  cmake -S . -B build
  cmake --build build
  echo "Build complete: build/pascc"
else
  make
  echo "Build complete: ./pascc"
fi
