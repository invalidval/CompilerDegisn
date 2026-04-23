#!/bin/bash
# 文档整合脚本
# 用途：将已有文档整合为最终课程设计报告

DOCS_DIR="docs"
FINAL_DIR="docs/Final-docs"

echo "开始整合课程设计报告..."

# 1. 复制需求分析（基本不需要修改）
echo "处理需求分析..."
cp "$DOCS_DIR/需求分析.md" "$FINAL_DIR/01-需求分析.md"

# 2. 复制总体设计（需要补充 record 类型说明）
echo "处理总体设计..."
cp "$DOCS_DIR/总体设计.md" "$FINAL_DIR/02-总体设计.md"

# 3. 复制详细设计（需要补充 record 类型详细设计）
echo "处理详细设计..."
cp "$DOCS_DIR/详细设计.md" "$FINAL_DIR/03-详细设计.md"

# 4. 复制 record 类型设计文档
echo "处理 record 类型扩展..."
cp "$DOCS_DIR/record类型实现设计.md" "$FINAL_DIR/04-Record类型扩展设计.md"

echo "基础文档复制完成！"
echo ""
echo "接下来需要手动完成："
echo "1. 在 02-总体设计.md 中补充 record 类型的架构说明"
echo "2. 在 03-详细设计.md 中引用 04-Record类型扩展设计.md"
echo "3. 创建 05-测试报告.md"
echo "4. 创建 06-使用说明.md"
echo "5. 创建 07-项目总结.md"
echo "6. 创建 08-个人总结.md（各成员填写）"
