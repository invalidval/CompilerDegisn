# Pascal-S Compiler Project Skeleton

本目录根据 `docs` 下的需求分析和总体设计文档建立，目标是实现：

`Pascal-S (.pas) -> 词法分析 -> 语法分析 -> 语义分析 -> 代码生成 -> C (.c)`

## 目录结构

```text
code/
├── CMakeLists.txt          # CMake 构建入口
├── Makefile                # 备用构建入口（Flex/Bison + g++）
├── include/
│   ├── common.h            # 公共类型定义
│   ├── token.h             # Token 结构定义
│   ├── symbol_table.h      # 符号表接口
│   ├── error_handler.h     # 错误处理接口
│   └── codegen_utils.h     # 代码生成辅助接口
├── src/
│   ├── main.cpp            # 程序入口
│   ├── lexer.l             # 词法分析（Flex）
│   ├── parser.y            # 语法分析（Bison）
│   ├── symbol_table.cpp    # 符号表实现
│   ├── error_handler.cpp   # 错误处理实现
│   └── codegen_utils.cpp   # 代码生成辅助实现
├── scripts/
│   └── build.sh            # 一键构建脚本
├── test/
│   ├── run_tests.sh        # 冒烟测试脚本
│   └── cases/
│       ├── valid/          # 合法样例
│       └── invalid/        # 非法样例
└── output/
	└── .gitkeep            # 目标输出目录
```

## 快速开始

```bash
cd code
bash scripts/build.sh
bash test/run_tests.sh
```

## 分工映射建议

- 词法分析：`src/lexer.l`
- 语法分析：`src/parser.y`
- 语义分析/符号表：`include/symbol_table.h` + `src/symbol_table.cpp`
- 代码生成：`include/codegen_utils.h` + `src/codegen_utils.cpp`
- 错误处理：`include/error_handler.h` + `src/error_handler.cpp`

当前是“可开发骨架”，尚未实现完整 Pascal-S 编译流程。
