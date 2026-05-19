```
code/
├── include/                        # 头文件
│   ├── ast.h                       # AST 节点定义
│   ├── symbol_table.h              # 符号表接口
│   ├── semantic_annotator.h        # 语义分析器接口
│   ├── code_generator.h            # 代码生成器接口
│   ├── codegen_utils.h             # 代码生成工具接口
│   ├── error_handler.h             # 错误处理接口
│   ├── token.h                     # Token 结构定义
│   ├── common.h                    # 公共类型定义
│   ├── debug_utils.h               # 调试工具接口
│   ├── parser_bridge.h             # 语法分析桥接
│   └── semantic_register.h         # 内置符号注册
├── src/                            # 源文件
│   ├── lexer.l                     # Flex 词法分析
│   ├── parser.y                    # Bison 语法分析
│   ├── main.cpp                    # 程序入口
│   ├── ast.cpp                     # AST 实现
│   ├── symbol_table.cpp            # 符号表实现
│   ├── semantic_annotator.cpp      # 语义分析实现
│   ├── code_generator.cpp          # 代码生成实现
│   ├── codegen_utils.cpp           # 代码生成工具实现
│   ├── error_handler.cpp           # 错误处理实现
│   ├── debug_utils.cpp             # 调试工具实现
│   └── semantic_register.cpp       # 内置符号注册实现
├── test/                           # 测试用例集
│   ├── run_tests.sh                # 冒烟测试（验证编译器可执行）
│   ├── run_all_tests.sh            # 编排全部测试脚本
│   ├── test_lexer_unit.sh          # 词法分析单元测试（31 项 Token 断言）
│   ├── test_semantic_unit.sh       # 语义分析 C++ 单元测试（19 项）
│   ├── run_parser_tests.sh         # 语法分析集成测试
│   ├── run_semantic_tests.sh       # 语义分析集成测试
│   ├── run_openset_check.sh        # Open Set 端到端测试（pascc + gcc）
│   ├── run_record_tests.sh         # Record 专项测试（编译 + 运行）
│   ├── verify_shell_scripts.sh     # Shell 脚本语法验证
│   ├── cases/                      # 各阶段测试用例
│   │   ├── valid/                  # 词法合法用例（30 个 .pas）
│   │   ├── invalid/                # 词法非法用例（17 个 .pas）
│   │   ├── parser_valid/           # 语法合法用例（8 个 .pas）
│   │   ├── parser_invalid/         # 语法非法用例（8 个 .pas）
│   │   ├── semantic_valid/         # 语义合法集成用例（26 个 .pas）
│   │   ├── semantic_invalid/       # 语义非法集成用例（29 个 .pas）
│   │   ├── record/                 # Record 文法扩展专项测试（22 个 .pas，
│   │   │                           #   19 正例 + 3 负例，含预期 .c 参考）
│   │   └── official/               # 课件标准示例（1 个 GCD 程序）
│   ├── open_set/                   # 开放测试集（70 个 .pas，
│   │                               #   基础运算到复杂算法全覆盖）
│   ├── semantic_unit/              # 语义分析 C++ 白盒测试（1 个 .cpp，
│   │                               #   ASTBuilder 构建 AST 直接测）
│   ├── semantic_stubs/             # 语义分析桩代码（绕过语法分析器）
│   ├── record_test_results/        # Record 测试运行结果日志
│   ├── .parser-test-out/           # 语法分析测试临时输出
│   ├── .semantic-test-out/         # 语义集成测试临时输出
│   └── .openset-out/               # Open Set 测试生成的 C 文件
├── docs/                           # 设计文档与课程报告
│   ├── Final-docs/                 # 最终版课程报告（.typ）
│   └── Meetings-record/            # 会议记录与问题汇报
├── scripts/                        # 构建与工具脚本
├── Makefile                        # GNU Make 构建
└── CMakeLists.txt                  # CMake 构建
```
