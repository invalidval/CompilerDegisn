#!/bin/bash

# Record 类型测试脚本
# 批量执行 test/cases/record/ 目录下的所有测试用例
# 记录编译结果和运行结果

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
RECORD_DIR="$SCRIPT_DIR/cases/record"
OUTPUT_DIR="$SCRIPT_DIR/record_test_results"
COMPILER="$PROJECT_ROOT/build/pascc"

# 检查编译器是否存在
if [ ! -f "$COMPILER" ]; then
    echo -e "${RED}错误: 编译器不存在: $COMPILER${NC}"
    echo "请先运行 bash scripts/build.sh 构建编译器"
    exit 1
fi

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

# 结果文件
RESULT_FILE="$OUTPUT_DIR/test_results_$(date +%Y%m%d_%H%M%S).txt"
SUMMARY_FILE="$OUTPUT_DIR/summary.txt"

# 统计变量
TOTAL=0
PASSED=0
FAILED=0
COMPILE_ERROR=0

echo "========================================" | tee "$RESULT_FILE"
echo "Record 类型测试报告" | tee -a "$RESULT_FILE"
echo "时间: $(date '+%Y-%m-%d %H:%M:%S')" | tee -a "$RESULT_FILE"
echo "========================================" | tee -a "$RESULT_FILE"
echo "" | tee -a "$RESULT_FILE"

# 遍历所有 .pas 文件
for pas_file in "$RECORD_DIR"/*.pas; do
    if [ ! -f "$pas_file" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    basename=$(basename "$pas_file" .pas)
    c_file="$RECORD_DIR/${basename}.c"
    exe_file="$RECORD_DIR/${basename}"

    echo -e "${BLUE}[测试 $TOTAL] $basename${NC}" | tee -a "$RESULT_FILE"
    echo "源文件: $pas_file" | tee -a "$RESULT_FILE"
    echo "" | tee -a "$RESULT_FILE"

    # 编译 Pascal 到 C
    echo ">>> 编译 Pascal -> C" | tee -a "$RESULT_FILE"
    if "$COMPILER" -i "$pas_file" -o "$c_file" > "$OUTPUT_DIR/${basename}_compile.log" 2>&1; then
        echo -e "${GREEN}✓ 编译成功${NC}" | tee -a "$RESULT_FILE"
        cat "$OUTPUT_DIR/${basename}_compile.log" | tee -a "$RESULT_FILE"

        # 编译 C 到可执行文件
        echo "" | tee -a "$RESULT_FILE"
        echo ">>> 编译 C -> 可执行文件" | tee -a "$RESULT_FILE"
        if gcc -o "$exe_file" "$c_file" -lm > "$OUTPUT_DIR/${basename}_gcc.log" 2>&1; then
            echo -e "${GREEN}✓ GCC 编译成功${NC}" | tee -a "$RESULT_FILE"

            # 运行可执行文件
            echo "" | tee -a "$RESULT_FILE"
            echo ">>> 运行程序" | tee -a "$RESULT_FILE"
            if timeout 5s "$exe_file" > "$OUTPUT_DIR/${basename}_output.log" 2>&1; then
                echo -e "${GREEN}✓ 运行成功${NC}" | tee -a "$RESULT_FILE"
                echo "输出:" | tee -a "$RESULT_FILE"
                cat "$OUTPUT_DIR/${basename}_output.log" | tee -a "$RESULT_FILE"
                PASSED=$((PASSED + 1))
            else
                exit_code=$?
                echo -e "${RED}✗ 运行失败 (退出码: $exit_code)${NC}" | tee -a "$RESULT_FILE"
                if [ -f "$OUTPUT_DIR/${basename}_output.log" ]; then
                    echo "输出:" | tee -a "$RESULT_FILE"
                    cat "$OUTPUT_DIR/${basename}_output.log" | tee -a "$RESULT_FILE"
                fi
                FAILED=$((FAILED + 1))
            fi
        else
            echo -e "${RED}✗ GCC 编译失败${NC}" | tee -a "$RESULT_FILE"
            cat "$OUTPUT_DIR/${basename}_gcc.log" | tee -a "$RESULT_FILE"
            COMPILE_ERROR=$((COMPILE_ERROR + 1))
        fi
    else
        echo -e "${RED}✗ Pascal 编译失败${NC}" | tee -a "$RESULT_FILE"
        cat "$OUTPUT_DIR/${basename}_compile.log" | tee -a "$RESULT_FILE"
        COMPILE_ERROR=$((COMPILE_ERROR + 1))
    fi

    echo "" | tee -a "$RESULT_FILE"
    echo "----------------------------------------" | tee -a "$RESULT_FILE"
    echo "" | tee -a "$RESULT_FILE"
done

# 输出统计信息
echo "" | tee -a "$RESULT_FILE"
echo "========================================" | tee -a "$RESULT_FILE"
echo "测试统计" | tee -a "$RESULT_FILE"
echo "========================================" | tee -a "$RESULT_FILE"
echo "总计: $TOTAL" | tee -a "$RESULT_FILE"
echo -e "${GREEN}通过: $PASSED${NC}" | tee -a "$RESULT_FILE"
echo -e "${RED}失败: $FAILED${NC}" | tee -a "$RESULT_FILE"
echo -e "${YELLOW}编译错误: $COMPILE_ERROR${NC}" | tee -a "$RESULT_FILE"

if [ $TOTAL -gt 0 ]; then
    success_rate=$(awk "BEGIN {printf \"%.2f\", ($PASSED/$TOTAL)*100}")
    echo "成功率: ${success_rate}%" | tee -a "$RESULT_FILE"
fi

echo "" | tee -a "$RESULT_FILE"
echo "详细结果已保存到: $RESULT_FILE" | tee -a "$RESULT_FILE"
echo "各测试日志保存在: $OUTPUT_DIR/" | tee -a "$RESULT_FILE"

# 保存摘要
cat > "$SUMMARY_FILE" <<EOF
Record 测试摘要 - $(date '+%Y-%m-%d %H:%M:%S')
========================================
总计: $TOTAL
通过: $PASSED
失败: $FAILED
编译错误: $COMPILE_ERROR
成功率: ${success_rate:-0}%
========================================
最新结果文件: $RESULT_FILE
EOF

echo ""
echo -e "${BLUE}摘要已保存到: $SUMMARY_FILE${NC}"

# 返回适当的退出码
if [ $FAILED -gt 0 ] || [ $COMPILE_ERROR -gt 0 ]; then
    exit 1
else
    exit 0
fi
