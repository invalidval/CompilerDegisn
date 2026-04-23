# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个 Pascal-S 到 C 语言的编译器项目，实现了完整的编译流程：词法分析 → 语法分析 → 语义分析 → 代码生成。

## 构建与测试命令

### 构建
```bash
# 使用构建脚本（推荐，自动选择 CMake 或 Makefile）
bash scripts/build.sh

# 或直接使用 Makefile
make

# 或使用 CMake
mkdir -p build && cd build
cmake ..
make
```

构建产物：`build/pascc` 或 `pascc`（根目录）

### 测试
```bash
# 基础冒烟测试
bash test/run_tests.sh

# 运行所有测试
bash test/run_all_tests.sh

# 词法分析单元测试
bash test/test_lexer_unit.sh

# 语义分析单元测试
bash test/test_semantic_unit.sh

# 语法分析测试
bash test/run_parser_tests.sh

# Close set 测试（核心测试集）
bash test/run_closeset_check.sh

# Open set 测试
bash test/run_openset_check.sh
```

### 编译器使用
```bash
# 基本用法
./build/pascc -i input.pas -o output.c

# 仅词法分析
./build/pascc -i input.pas --lex

# 输出 token 流
./build/pascc -i input.pas --dump-tokens

# 仅语法分析
./build/pascc -i input.pas --parse

# 仅语义分析
./build/pascc -i input.pas --semantic

# 输出注解后的 AST
./build/pascc -i input.pas --dump-annotated-ast

# 使用 TUI 可视化编译过程
bash scripts/run_tui.sh -i test/close_set/00_main.pas
bash scripts/run_tui.sh -i test/close_set/00_main.pas --log-level debug
```

## 架构设计

### 编译流程
```
Pascal-S 源码 (.pas)
    ↓
词法分析器 (Lexer - Flex)
    ↓ Token 流
语法分析器 (Parser - Bison)
    ↓ AST
语义分析器 (Semantic Annotator)
    ↓ 注解后的 AST
代码生成器 (Code Generator - Visitor 模式)
    ↓
C 代码 (.c)
```

### 核心模块

#### 1. 词法分析 (src/lexer.l)
- 使用 Flex 实现
- 关键特性：
  - 标识符统一转换为小写（Pascal-S 不区分大小写）
  - 支持 `{ }` 注释处理
  - 识别关键字、标识符、常量（整数、实数、字符、布尔）
  - 错误恢复机制

#### 2. 语法分析 (src/parser.y)
- 使用 Bison (LALR(1)) 实现
- 在归约时构建 AST，不直接生成目标代码
- 使用 `ASTBuilder` (ast.cpp) 创建节点
- 支持错误恢复（panic mode）

#### 3. AST (include/ast.h, src/ast.cpp)
- 核心节点类型：
  - `ProgramNode`: 程序根节点
  - `BlockNode`: 代码块（声明 + 语句）
  - 声明节点：`VarDeclNode`, `ConstDeclNode`, `ProcDeclNode`, `FuncDeclNode`, `ParamDeclNode`
  - 语句节点：`AssignStmtNode`, `IfStmtNode`, `WhileStmtNode`, `ForStmtNode`, `CompoundStmtNode`, `ProcCallNode`, `BreakStmtNode`
  - 表达式节点：`BinaryExprNode`, `UnaryExprNode`, `IdentifierNode`, `LiteralNode`, `ArrayAccessNode`
  - 类型节点：`ArrayTypeNode`
  - 通用节点：`ListNode` (用于各种列表结构)

- 每个节点包含：
  - `nodeType`: 节点类型
  - `dataType`: 数据类型（语义分析阶段填充）
  - `pos`: 源码位置（行列号）
  - `children`: 子节点列表
  - `symbolEntry`: 符号表条目引用（语义分析阶段绑定）

#### 4. 符号表 (include/symbol_table.h, src/symbol_table.cpp)
- 采用"哈希表 + 栈"结构
- 支持两层作用域：全局 (Level 0) + 局部 (Level 1)
- **约束**：过程/函数不可嵌套定义
- 符号条目 (`SymbolEntry`) 包含：
  - 名称（统一小写）
  - 类型 (`DataType`)
  - 种类（变量/常量/数组/过程/函数）
  - 作用域层级
  - 常量值、数组边界、参数列表等属性

- 关键接口：
  - `enterScope()`: 进入新作用域
  - `exitScope()`: 退出当前作用域
  - `insert()`: 插入符号
  - `lookup()`: 查找符号（支持作用域链）

#### 5. 语义分析 (include/semantic_annotator.h, src/semantic_annotator.cpp)
- 递归遍历 AST，执行：
  - 类型检查（赋值、表达式、参数）
  - 作用域检查（未声明、重复定义）
  - 数组下标检查
  - 参数传递检查（值传递 vs 引用传递 `var`）
  - 函数返回值检查
  - `break` 语句上下文检查（必须在循环内）
  - 过程嵌套检查

- 为 AST 节点注解：
  - `dataType`: 推导的数据类型
  - `symbolEntry`: 绑定符号表条目

#### 6. 代码生成 (include/code_generator.h, src/code_generator.cpp)
- 基于 Visitor 模式遍历注解后的 AST
- 关键映射规则：
  - Pascal 类型 → C 类型：`integer` → `int`, `real` → `double`, `boolean` → `int`, `char` → `char`
  - 数组下标偏移：Pascal 数组可以从任意整数开始（如 `array[3..9]`），C 从 0 开始，需要生成偏移计算
  - `var` 参数：Pascal 引用传递 → C 指针传递
  - 过程 → C void 函数
  - 函数 → C 函数（带返回值）

- 代码分区：
  - `globalDecls_`: 全局变量声明
  - `prototypes_`: 函数原型声明
  - `definitions_`: 函数定义
  - `mainBody_`: main 函数体

#### 7. 错误处理 (include/error_handler.h, src/error_handler.cpp)
- 统一的错误报告接口
- 支持词法、语法、语义、代码生成各阶段错误
- 错误恢复策略：记录所有错误但不立即终止，尽可能多地报告问题
- 错误格式：`Error [Line:Col]: Description`

### 关键约束与特性

1. **标识符大小写不敏感**：词法阶段统一转小写，符号表 Key 全为小写
2. **过程不可嵌套**：最大作用域深度为 2（全局 + 局部）
3. **数组下标任意起始**：支持 `array[3..9]` 等非零起始数组
4. **参数传递**：支持值传递和引用传递（`var` 参数）
5. **注释**：支持 `{ }` 形式的注释
6. **内置过程**：`read`, `write`

### 测试结构

- `test/cases/valid/`: 合法测试用例
- `test/cases/invalid/`: 非法测试用例
- `test/cases/parser_valid/`: 语法分析合法用例
- `test/cases/parser_invalid/`: 语法分析非法用例
- `test/close_set/`: 核心测试集（约 200+ 用例）
- `test/open_set/`: 开放测试集（约 150+ 用例）
- `test/semantic_unit/`: 语义分析单元测试
- `test/semantic_stubs/`: 语义分析桩代码

### TUI 可视化

项目包含一个可选的 TUI（终端用户界面）用于可视化编译过程：
- `scripts/pascc_tui.py`: 基础 TUI
- `scripts/pascc_tui_interactive.py`: 交互式 TUI
- `scripts/run_tui.sh`: TUI 启动脚本

通过环境变量 `PASCC_EVENT_STREAM=1` 启用事件流输出，TUI 解析事件流并实时显示编译阶段进度。

### 文件组织

```
code/
├── include/          # 头文件
│   ├── ast.h         # AST 节点定义
│   ├── semantic_annotator.h
│   ├── code_generator.h
│   ├── symbol_table.h
│   ├── error_handler.h
│   ├── codegen_utils.h
│   ├── debug_utils.h
│   └── ...
├── src/              # 源文件
│   ├── lexer.l       # Flex 词法定义
│   ├── parser.y      # Bison 语法定义
│   ├── main.cpp      # 程序入口
│   ├── ast.cpp
│   ├── semantic_annotator.cpp
│   ├── code_generator.cpp
│   ├── symbol_table.cpp
│   ├── error_handler.cpp
│   └── ...
├── test/             # 测试用例
├── scripts/          # 构建和工具脚本
├── docs/             # 设计文档
├── build/            # 构建产物（生成）
└── output/           # 输出目录
```

## 开发注意事项

### 修改词法规则
- 编辑 `src/lexer.l`
- 重新运行 `bash scripts/build.sh` 或 `make`

### 修改语法规则
- 编辑 `src/parser.y`
- 注意维护 AST 构建逻辑
- 重新构建

### 添加语义检查
- 在 `src/semantic_annotator.cpp` 中添加检查逻辑
- 确保错误信息包含准确的行列号（使用 `node->pos`）

### 修改代码生成
- 在 `src/code_generator.cpp` 中修改 Visitor 方法
- 注意数组下标偏移和 `var` 参数的正确处理

### 调试技巧
- 使用 `--dump-tokens` 查看 token 流
- 使用 `--parse` 查看 AST 构建是否正确
- 使用 `--dump-annotated-ast` 查看语义注解结果
- 使用 TUI 可视化编译过程
- 查看 `include/debug_utils.h` 中的调试工具函数
