#!/usr/bin/env bash
set -euo

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

mkdir -p build

CXX="${CXX:-g++}"
$CXX -std=c++17 -Wall -Wextra -Iinclude \
  test/semantic_unit/semantic_unit.cpp \
  src/ast.cpp \
  src/semantic_annotator.cpp \
  src/symbol_table.cpp \
  src/error_handler.cpp \
  -o build/semantic_unit_tests

./build/semantic_unit_tests
