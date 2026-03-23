#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

mkdir -p "$BUILD_DIR"

# 固定桩 + 语义开发核心源码一并编译，屏蔽语法分析实现。
g++ \
    -std=c++17 \
    -Wall -Wextra \
    -I"$PROJECT_ROOT/include" \
    "$SCRIPT_DIR/stub.cpp" \
    "$PROJECT_ROOT/src/ast.cpp" \
    "$PROJECT_ROOT/src/semantic_annotator.cpp" \
    "$PROJECT_ROOT/src/symbol_table.cpp" \
    "$PROJECT_ROOT/src/error_handler.cpp" \
    -o "$BUILD_DIR/semantic_stub"

echo "编译成功: $BUILD_DIR/semantic_stub"

"$BUILD_DIR/semantic_stub"