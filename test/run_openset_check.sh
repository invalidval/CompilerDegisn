#!/usr/bin/env bash
set -uo

# 自动批量测试 open_set 下所有 .pas 文件
# 1. 使用 pascc 编译 .pas -> .c（记录 pascc 报错）
# 2. 对 pascc 成功的用例，使用 gcc 编译生成的 .c（记录 gcc 报错）

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

OPENSET_DIR="$SCRIPT_DIR/open_set"
OUT_DIR="$SCRIPT_DIR/.openset-out"
PASCC_LOG="$SCRIPT_DIR/failed_pascc_cases.log"
GCC_LOG="$SCRIPT_DIR/failed_gcc_cases.log"
GCC_WARN_LOG="$SCRIPT_DIR/gcc_warning_cases.log"

mkdir -p "$OUT_DIR"

# 查找 pascc 二进制文件
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
    echo "[ERROR] pascc 编译器未找到，请先编译: bash scripts/build.sh"
    exit 1
fi

# 检查 gcc 是否可用
if ! command -v gcc >/dev/null 2>&1; then
    echo "[ERROR] gcc 未找到，请安装 gcc"
    exit 1
fi

> "$PASCC_LOG"
> "$GCC_LOG"
> "$GCC_WARN_LOG"

pascc_total=0
pascc_pass=0
pascc_fail=0
gcc_total=0
gcc_pass=0
gcc_fail=0
gcc_warn=0

echo "========================================="
echo "[openset] testing $(find "$OPENSET_DIR" -maxdepth 1 -name '*.pas' 2>/dev/null | wc -l | tr -d ' ') files..."
echo "========================================="

for pas in "$OPENSET_DIR"/*.pas; do
    [[ -e "$pas" ]] || continue
    name=$(basename "$pas" .pas)
    pascc_total=$((pascc_total + 1))

    c_file="$OUT_DIR/${name}.c"
    stderr_file="$OUT_DIR/${name}.pascc_stderr"

    echo -n "[$pascc_total] $name.pas ... "

    # Step 1: pascc 编译 .pas -> .c
    set +e
    if command -v timeout >/dev/null 2>&1; then
        timeout 30s "$BIN" -i "$pas" -o "$c_file" >/dev/null 2>"$stderr_file"
        code=$?
        timed_out=0
        [[ $code -eq 124 ]] && timed_out=1
    else
        "$BIN" -i "$pas" -o "$c_file" >/dev/null 2>"$stderr_file"
        code=$?
        timed_out=0
    fi
    set -e

    if [[ $timed_out -eq 1 ]]; then
        echo "pascc TIMEOUT"
        pascc_fail=$((pascc_fail + 1))
        echo "=== $name.pas ===" >> "$PASCC_LOG"
        echo "exit_code=$code (timeout)" >> "$PASCC_LOG"
        cat "$stderr_file" >> "$PASCC_LOG"
        echo "----------------------" >> "$PASCC_LOG"
        continue
    fi

    if [[ $code -ne 0 ]]; then
        echo "pascc FAIL"
        pascc_fail=$((pascc_fail + 1))
        echo "=== $name.pas ===" >> "$PASCC_LOG"
        echo "exit_code=$code" >> "$PASCC_LOG"
        cat "$stderr_file" >> "$PASCC_LOG"
        echo "----------------------" >> "$PASCC_LOG"
        continue
    fi

    pascc_pass=$((pascc_pass + 1))

    # Step 2: gcc 编译生成的 .c 文件
    gcc_total=$((gcc_total + 1))
    gcc_stderr="$OUT_DIR/${name}.gcc_stderr"

    set +e
    if command -v timeout >/dev/null 2>&1; then
        timeout 30s gcc -std=c99 -Wall -Wextra -c "$c_file" -o /dev/null 2>"$gcc_stderr"
        gcc_code=$?
        gcc_timed_out=0
        [[ $gcc_code -eq 124 ]] && gcc_timed_out=1
    else
        gcc -std=c99 -Wall -Wextra -c "$c_file" -o /dev/null 2>"$gcc_stderr"
        gcc_code=$?
        gcc_timed_out=0
    fi
    set -e

    if [[ $gcc_timed_out -eq 1 ]]; then
        echo "gcc TIMEOUT"
        gcc_fail=$((gcc_fail + 1))
        echo "=== $name.pas (generated: $name.c) ===" >> "$GCC_LOG"
        echo "exit_code=$gcc_code (timeout)" >> "$GCC_LOG"
        cat "$gcc_stderr" >> "$GCC_LOG"
        echo "----------------------" >> "$GCC_LOG"
    elif [[ $gcc_code -ne 0 ]]; then
        echo "gcc ERROR"
        gcc_fail=$((gcc_fail + 1))
        echo "=== $name.pas (generated: $name.c) ===" >> "$GCC_LOG"
        echo "exit_code=$gcc_code" >> "$GCC_LOG"
        cat "$gcc_stderr" >> "$GCC_LOG"
        echo "----------------------" >> "$GCC_LOG"
    elif [[ -s "$gcc_stderr" ]]; then
        echo "gcc WARN"
        gcc_warn=$((gcc_warn + 1))
        echo "=== $name.pas (generated: $name.c) ===" >> "$GCC_WARN_LOG"
        cat "$gcc_stderr" >> "$GCC_WARN_LOG"
        echo "----------------------" >> "$GCC_WARN_LOG"
    else
        echo "OK"
        gcc_pass=$((gcc_pass + 1))
    fi
done

echo ""
echo "========================================="
echo "[openset] summary"
echo "========================================="
echo "pascc: ${pascc_pass} passed, ${pascc_fail} failed, ${pascc_total} total"
echo "gcc:   ${gcc_pass} clean, ${gcc_warn} warnings, ${gcc_fail} errors, ${gcc_total} total"

if [[ $pascc_fail -gt 0 ]]; then
    echo "  pascc 失败的用例: $PASCC_LOG"
fi
if [[ $gcc_fail -gt 0 ]]; then
    echo "  gcc 编译错误的用例: $GCC_LOG"
fi
if [[ $gcc_warn -gt 0 ]]; then
    echo "  gcc 编译警告的用例: $GCC_WARN_LOG"
fi

if [[ $pascc_fail -eq 0 ]] && [[ $gcc_fail -eq 0 ]]; then
    echo ""
    echo "All openset tests passed."
    exit 0
fi

echo ""
exit 1
