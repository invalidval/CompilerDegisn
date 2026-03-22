#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

if command -v cmake >/dev/null 2>&1; then
  BISON_BIN=""
  if [[ -x "/opt/homebrew/opt/bison/bin/bison" ]]; then
    BISON_BIN="/opt/homebrew/opt/bison/bin/bison"
  elif command -v bison >/dev/null 2>&1; then
    BISON_BIN="$(command -v bison)"
  fi

  CMAKE_ARGS=(
    -S "$PROJECT_ROOT"
    -B "$BUILD_DIR"
  )
  if [[ -n "$BISON_BIN" ]]; then
    CMAKE_ARGS+=("-DBISON_EXECUTABLE=$BISON_BIN")
  fi

  echo "[build] Configure with CMake"
  cmake "${CMAKE_ARGS[@]}"

  echo "[build] Compile with CMake"
  JOBS=1
  if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
  elif command -v sysctl >/dev/null 2>&1; then
    JOBS="$(sysctl -n hw.ncpu)"
  fi
  cmake --build "$BUILD_DIR" -j"$JOBS"

  if [[ -x "$BUILD_DIR/pascc" ]]; then
    echo "[build] Build complete: $BUILD_DIR/pascc"
  else
    echo "[build] Build finished, but binary not found at $BUILD_DIR/pascc"
    exit 1
  fi
else
  echo "[build] CMake not found, fallback to Makefile"
  (cd "$PROJECT_ROOT" && make)
  echo "[build] Build complete: $PROJECT_ROOT/pascc"
fi
