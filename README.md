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

## 编译过程可视化终端（可选）

默认 `pascc` 命令行行为不变。只有在你主动运行 TUI 包装器时，才会显示编译阶段可视化。

```bash
# 先构建
bash scripts/build.sh

# 启动可视化终端（示例）
bash scripts/run_tui.sh -i test/close_set/00_main.pas

# 调高日志细节（可选）
bash scripts/run_tui.sh -i test/close_set/00_main.pas --log-level debug
```

说明：

- 该模式通过环境变量 `PASCC_EVENT_STREAM=1` 启用结构化事件流，仅影响当前 TUI 进程。
- 该模式通过环境变量 `PASCC_LOG_LEVEL` 控制过程日志粒度（`off/error/warn/info/debug`）。
- 普通执行（如 `./build/pascc -i xxx.pas`）不会显示任何额外 UI。
- TUI 中按 `q` 可中止编译，编译结束后按 `Enter` 或 `q` 退出界面。

## 分工映射建议

- 词法分析：`src/lexer.l`
- 语法分析：`src/parser.y`
- 语义分析/符号表：`include/symbol_table.h` + `src/symbol_table.cpp`
- 代码生成：`include/codegen_utils.h` + `src/codegen_utils.cpp`
- 错误处理：`include/error_handler.h` + `src/error_handler.cpp`

当前是“可开发骨架”，尚未实现完整 Pascal-S 编译流程。
