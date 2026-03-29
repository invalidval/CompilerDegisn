#!/usr/bin/env bash
set -euo

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
echo "Root dir: $ROOT_DIR"
cd "$ROOT_DIR"

echo "Building..."
bash scripts/build.sh

echo "Running lexer tests..."
bash test/test_lexer_unit.sh

echo "Running parser tests..."
bash test/run_parser_tests.sh

echo "Running semantic tests..."
bash test/test_semantic_unit.sh

echo "Running full tests..."
bash test/run_tests.sh
bash test/run_openset_check.sh

echo "All tests finished."
