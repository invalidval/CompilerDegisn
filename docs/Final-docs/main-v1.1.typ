#import "typst/lib.typ": experiment-report, styled-parameter-table, algorithm
#import "@preview/cuti:0.3.0": show-cn-fakebold
#show: show-cn-fakebold

#show: doc => experiment-report(
  row1: "",
  row2: "",
  lab: "",
  class: "",
  name: "",
  student-id: "",
  major: "",
  date: "",
  doc
)

#show cite: it => super(it)

#outline(
  // title: none
)

#pagebreak()

= 课程设计任务和目标

本项目是编译原理与技术课程的课程设计，目标是设计并实现一个完整的编译程序，将 Pascal-S（Pascal 语言的一个教学子集）源代码翻译为 C 语言目标代码。编译器采用经典的编译前端架构，依次完成词法分析、语法分析、语义分析和代码生成四个阶段。

== 任务目标

- 实现完整的 Pascal-S 到 C 语言的编译流程，生成可编译运行的 C 代码。
- 采用 Flex 实现词法分析器，Bison 实现 LALR(1) 语法分析器。
- 在语法分析阶段构建抽象语法树（AST），语义分析阶段对 AST 进行类型推导和注解，代码生成阶段基于 Visitor 模式遍历注解后的 AST 生成目标代码。
- 支持 Pascal-S 的核心语法特性：基本类型、数组、过程/函数、参数传递（值传递与引用传递）、控制流语句。
- 扩展支持 record 类型（结构体），包括 record 变量的字段访问、record 作为参数、record 数组等。

== 项目分工

本项目由五名成员协作完成，分工如下：

#styled-parameter-table(
  columns: 3,
  [角色], [姓名], [职责],
  [语法分析], [王嘉晗], [AST 契约定义、Bison 建树动作、解析结果导出、单元测试],
  [组长/语义分析], [张宸宇], [统筹架构、模块联调、集成测试；符号表实现、AST 语义注解、类型检查、语义错误处理、单元测试],
  [词法分析], [胡航宾], [Flex 词法规则、Token 产出、行列号维护、词法错误检测、单元测试],
  [代码生成（基础）], [李思远], [Visitor 基础遍历、基础语句生成、代码生成框架],
  [代码生成（进阶）], [谢康], [数组访问、过程调用、传引用参数、复杂节点代码生成],
)

== 开发环境

- 操作系统：macOS / Windows / Linux
- 编译器：g++（C++17 标准）
- 词法/语法工具：Flex 2.6.4 / Bison 3.8.2
- 构建工具：GNU Make / CMake
- 版本控制：Git
- 代码统计工具：cloc

= 需求分析

== 数据流图

编译器整体数据流如下图所示：Pascal-S 源文件输入后，依次经过词法分析、语法分析、语义分析、代码生成四个阶段，最终输出 C 语言源文件。

#figure(
  image("typst/assets/image-1.png", width: 95%),
  caption: [编译器数据流图]
)

== 源语言规范

=== 词法规范

Pascal-S 的词法规则遵循标准 Pascal 规范，关键特征如下：

1. *标识符*：以字母开头，后跟字母或数字。不区分大小写，词法阶段统一转换为小写。
2. *关键字*：共 33 个保留字，包括 `program`、`begin`、`end`、`const`、`var`、`type`、`record`、`procedure`、`function`、`if`、`then`、`else`、`while`、`do`、`for`、`to`、`downto`、`break`、`read`、`write`、`array`、`of`、`integer`、`real`、`boolean`、`char`、`true`、`false`、`not`、`and`、`or`、`div`、`mod`。
3. *常量*：支持整数、实数（含科学计数法）、十六进制数（`0x` 和 `$` 前缀）、字符常量（单引号）、字符串常量。
4. *运算符*：赋值 `:=`，关系比较（`=`、`<>`、`<`、`>`、`<=`、`>=`），算术运算（`+`、`-`、`*`、`/`、`div`、`mod`），逻辑运算（`and`、`or`、`not`）。
5. *注释*：支持 `{ }` 风格的注释，且支持嵌套注释。注释可出现在任何单词之后。
6. *编译指令*：支持 `{$...}` 形式的编译器指令（如 `{$IFDEF}`、`{$ENDIF}`）。

=== 语法规范

编译器采用 LALR(1) 分析方法，接受完整的 Pascal-S 文法。程序主体结构如下：

- *程序结构*：`program id (idlist) ; program_body .`——程序头后跟常量声明、类型声明、变量声明、子程序声明和复合语句。
- *声明顺序*：常量声明 `const ...` → 类型声明 `type ...` → 变量声明 `var ...` → 子程序声明 `procedure/function ...`。
- *类型系统*：支持 `integer`、`real`、`boolean`、`char` 四种基本类型，以及 `array [lower..upper] of type` 数组类型，和扩展的 `record ... end` 记录类型。
- *子程序*：支持过程和函数定义，参数支持值传递和引用传递（`var` 参数）。过程和函数不允许嵌套定义。
- *语句*：支持赋值、过程调用、复合语句（`begin...end`）、条件语句（`if-then-else`）、`for` 循环（`to`/`downto`）、`while` 循环、`break` 语句。
- *表达式*：支持算术、关系、逻辑运算，以及函数调用表达式。
- *变量访问*：支持标识符、数组下标（`a[i]`）、字段访问（`r.field`）。

=== 语义规范

编译器在语义分析阶段实施以下检查：

1. *类型系统*：
   - `integer` 映射为 C 的 `int`
   - `real` 映射为 C 的 `float`
   - `boolean` 映射为 C 的 `int`（0 表示 false，非 0 表示 true）
   - `char` 映射为 C 的 `char`
   - `record` 映射为 C 的 `typedef struct`

2. *作用域规则*：采用两层作用域（全局作用域 + 局部作用域）。过程/函数由于不可嵌套定义，局部作用域最多一层。内层可访问外层变量，同名局部变量屏蔽外层变量。

3. *参数传递*：
   - 值传递：复制实参值，C 实现为普通参数。
   - 引用传递（`var`）：传递地址，C 实现为指针参数，调用时传地址 `&arg`，使用需解引用 `*arg`。

4. *数组处理*：Pascal-S 数组可以从任意整数开始（如 `array[3..9] of integer`），而 C 数组从 0 开始，编译器需自动计算下标偏移：`a[i]` → `a[i - lower]`。

5. *类型检查*：
   - 变量/函数使用前必须声明。
   - 同一作用域内名字不可重复定义。
   - 赋值语句左右类型兼容（允许 `integer → real` 隐式转换）。
   - 数组下标表达式必须为 `integer` 类型。
   - 过程/函数调用参数个数和类型必须匹配。
   - `break` 语句必须在循环体内。
   - 函数结果赋值（`func_name := expr`）只允许出现在对应函数体内。

6. *Record 类型扩展*：
   - 支持类型声明 `type T = record ... end;`
   - 支持字段访问 `rec_var.field_name`
   - 字段访问可作为赋值左值
   - 支持 record 作为函数/过程参数
   - 支持 record 数组

== 功能模块需求

编译器划分为以下核心功能模块：

#styled-parameter-table(
  columns: (auto, auto, auto),
  [模块], [实现文件], [功能说明],
  [词法分析器], [src/lexer.l], [Flex 实现，负责 Token 识别、标识符大小写归一化、注释处理、行列号维护、词法错误检测],
  [语法分析器], [src/parser.y], [Bison 实现，LALR(1) 分析，归约时构建 AST，恐慌模式错误恢复],
  [AST 数据结构], [include/ast.h, src/ast.cpp], [20 种 AST 节点类型，Visitor 模式访问接口，Arena 内存管理],
  [符号表], [include/symbol_table.h, src/symbol_table.cpp], [哈希表+栈结构，两层作用域管理，支持变量/常量/过程/函数/类型别名],
  [语义分析器], [include/semantic_annotator.h, src/semantic_annotator.cpp], [递归遍历 AST，类型推导与检查，符号绑定，语义错误报告],
  [代码生成器], [include/code_generator.h, src/code_generator.cpp], [Visitor 模式遍历注解后 AST，分四区收集代码，生成完整 C 程序],
  [代码生成工具], [include/codegen_utils.h, src/codegen_utils.cpp], [类型映射、声明生成、读写语句、代码组装等辅助函数],
  [错误处理], [include/error_handler.h, src/error_handler.cpp], [统一错误报告接口，记录行号列号，错误恢复策略],
  [主程序], [src/main.cpp], [命令行解析，编译流程控制，事件流输出，调试支持],
)

== 非功能性需求

- 编译速度：千行以内 Pascal-S 代码应在数秒内完成编译。
- 内存管理：采用 Arena 模式管理 AST 节点和符号表条目生命周期，避免内存泄漏。
- 错误恢复：各阶段遇到错误不应立即终止，应尽可能多地收集并报告错误。语法分析采用恐慌模式在分号处恢复。
- 可扩展性：Visitor 模式使代码生成逻辑与 AST 结构解耦，便于新增节点类型和语义检查。

#pagebreak()

= 总体设计

== 系统架构

编译器采用经典的"词法分析 → 语法分析 → 语义分析 → 代码生成"四阶段流水线架构。各阶段通过中间表示进行数据传递：

#figure(
  image("typst/assets/image-2.png", width: 89%),
  caption: [编译器总体结构图]
)

编译流程如下：

+ Pascal-S 源文件（`.pas`）首先由 Flex 词法分析器进行词法分析，输出 Token 流。
+ Token 流由 Bison 语法分析器消费，进行 LALR(1) 分析，在归约过程中调用 ASTBuilder 构建抽象语法树。
+ 语法分析输出的 AST 根节点（`ProgramNode`）传递给语义分析器。
+ 语义分析器递归遍历 AST，进行类型推导、符号绑定和各类语义检查，为 AST 节点注解 `dataType` 和 `symbolEntry`。
+ 代码生成器基于 Visitor 模式遍历注解后的 AST，分四个缓冲区收集代码，最终由 `CodegenUtils::wrapAsCProgram` 组装为完整 C 程序输出。

== 核心数据结构设计

=== AST 节点体系

抽象语法树是整个编译器的核心数据结构，定义了统一的 `ASTNode` 基类及 25 种派生节点类型：

#styled-parameter-table(
  columns: (28%, 22%, 50%),
  [节点类], [NodeType], [对应文法结构],
  [ProgramNode], [Program], [程序根节点，包含程序名和 Body],
  [BlockNode], [Block], [作用域块，子节点：consts/types/vars/subs/compound],
  [VarDeclNode], [VarDecl], [变量声明：idlist : type],
  [ConstDeclNode], [ConstDecl], [常量声明：id = const_value],
  [ProcDeclNode], [ProcDecl], [过程声明：procedure id(params); body],
  [FuncDeclNode], [FuncDecl], [函数声明：function id(params): type; body],
  [AssignStmtNode], [AssignStmt], [赋值语句：var := expr],
  [IfStmtNode], [IfStmt], [条件语句：if expr then stmt else stmt],
  [WhileStmtNode], [WhileStmt], [while 循环：while expr do stmt],
  [ForStmtNode], [ForStmt], [for 循环：for id := expr to/downto expr do stmt],
  [BreakStmtNode], [BreakStmt], [break 语句],
  [CompoundStmtNode], [CompoundStmt], [复合语句：begin stmt_list end],
  [ProcCallNode], [ProcCall], [过程/函数调用：id(args)],
  [BinaryExprNode], [BinaryExpr], [二元表达式：expr op expr],
  [UnaryExprNode], [UnaryExpr], [一元表达式：op expr],
  [IdentifierNode], [Identifier], [标识符引用],
  [LiteralNode], [Literal], [字面量],
  [ArrayAccessNode], [ArrayAccess], [数组下标访问：arr[index]],
  [ArrayTypeNode], [ArrayType], [数组类型定义：array[lower..upper] of type],
  [ParamDeclNode], [ParamDecl], [参数声明：var idlist : type],
  [TypeDeclNode], [TypeDecl], [类型声明：type name = typedef],
  [RecordTypeNode], [RecordType], [record 类型定义：record fields end],
  [FieldDeclNode], [FieldDecl], [字段声明：idlist : type],
  [FieldAccessNode], [FieldAccess], [字段访问：base.field],
  [ListNode], [List], [通用列表容器，ListKind 区分标识符/语句/表达式/参数等列表],
)

ASTNode 基类包含以下核心字段：

- `nodeType`：节点类型枚举值（`NodeType`）。
- `dataType`：推导出的数据类型（`DataType`），初始值为 `Unknown`，语义分析阶段填充。
- `pos`：源码位置（`SourcePos`），包含行号和列号。
- `children`：子节点指针的向量（`std::vector<ASTNode*>`）。
- `symbolEntry`：符号表条目指针，语义分析阶段绑定。

ListNode 是特殊的通用容器节点，通过 `ListKind` 枚举区分用途：`Identifiers`（标识符列表）、`Statements`（语句列表）、`Expressions`（表达式列表）、`Parameters`（参数列表）、`Declarations`（声明列表）、`ArrayRanges`（多维数组范围）、`FieldAccess`（字段访问链）。

AST 对象通过 `ASTBuilder` 类进行统一创建和管理。ASTBuilder 内部维护一个 `std::vector<std::unique_ptr<ASTNode>>` Arena，所有节点通过模板方法 `create<T>(...)` 创建后追加到 Arena 中。这种设计确保了：
- 指针稳定性：所有节点在 Arena 生命周期内地址不变。
- 内存安全：Arena 析构时自动释放所有节点。
- 使用便利性：调用者无需关心内存管理。

=== 符号表

符号表采用"哈希表 + 栈"结构实现，用于管理编译过程中的符号信息：

- 底层数据结构：`std::vector<std::unordered_map<std::string, const SymbolEntry*>>`
- 每个作用域对应一个哈希表（`unordered_map`）。
- 作用域以栈方式管理：`enterScope()` 压入新表，`exitScope()` 弹出。
- 符号名 Key 统一经 `normalizeName()` 转为小写。
- 插入时写入当前作用域，查找（`lookup()`）从当前作用域向全局方向遍历。

符号条目 `SymbolEntry` 包含以下字段：

#styled-parameter-table(
  columns: (25%, 25%, 50%),
  [字段], [类型], [说明],
  [name], [string], [符号名称（小写）],
  [kind], [SymbolKind], [符号种类：Variable/Constant/Procedure/Function/Parameter/TypeAlias],
  [type], [DataType], [数据类型：Integer/Real/Boolean/Char/Record等],
  [scopeLevel], [int], [作用域层级（0=全局，1=局部）],
  [isArray], [bool], [是否为数组类型],
  [arrayBounds], [vector<ArrayBound>], [多维数组各维的上下界],
  [hasConstLiteral], [bool], [是否是常量字面量],
  [constLiteralText], [string], [常量字面量文本],
  [isStringLikeConst], [bool], [是否为类字符串常量],
  [params], [vector<ParamInfo>], [过程/函数的参数列表],
  [isVarParam], [bool], [是否为 var 引用参数],
  [fields], [vector<ParamInfo>], [Record 类型的字段列表],
  [typeName], [string], [用户自定义类型的名称],
)

SymbolEntry 使用静态工厂方法创建：`makeVariable()`、`makeConstant()`、`makeProcedure()`、`makeFunction()`、`makeParameter()`、`makeTypeAlias()`。

符号表的 `entryArena_`（`std::vector<std::unique_ptr<SymbolEntry>>`）确保符号条目生命周期与符号表一致，即使作用域退出后指针仍然有效，便于代码生成阶段回溯引用。

=== Token 元信息

项目中定义了 `include/token.h` 作为统一的 Token 描述结构，用于表达 `type/lexeme/line/column` 四类基础信息；在主编译链路中，词法阶段并不是以 `Token` 对象数组的形式向前传递数据，而是由 Flex 直接返回 Bison Token 编号，并通过 `yylval`、`yylloc` 以及词法模块查询函数暴露 Token 元信息。字段如下：

#styled-parameter-table(
  columns: (22%, 28%, 50%),
  [字段], [位置], [说明],
  [token kind], [`yylex()` 返回值], [关键字、标识符、数字、字符、字符串、多字符运算符或单字符 ASCII Token],
  [lexeme], [`lexerLastLexeme()` / `yytext`], [最近一次识别到的词素文本],
  [type label], [`lexerLastType()`], [用于 `--lex` / `--dump-tokens` 输出的分类标签，如 `Keyword`、`Identifier`、`Number`],
  [rule label], [`lexerLastRule()`], [规则命中标签，如 `keyword_identifier`、`real_number`、`assign_operator`],
  [line / column], [`yylloc` / `lexerLastLine()` / `lexerLastColumn()`], [Token 起始行列号，采用 1-based 计数],
)

=== Visitor 模式

编译器采用 Visitor 模式实现 AST 遍历逻辑的分离。`ASTVisitor` 定义纯虚接口，为每种节点类型提供 `visit()` 方法。`SemanticAnnotator` 和 `CodeGenerator` 均实现该接口，分别实现语义注解和代码生成两个正交关注点。

== 功能模块划分

=== 模块功能

各功能模块的职责如下：

1. *词法分析模块（Lexer）*：
   - 使用 Flex 实现，定义在 `src/lexer.l`。
   - 识别所有 Token 类型并返回给 Bison 语法分析器。
   - 标识符统一转换为小写。
   - 跟踪行号和列号，为每个 Token 提供精确的位置信息。
   - 检测并报告词法错误（非法字符、未闭合注释、数值溢出等）。
   - 支持嵌套注释和编译器指令。

2. *语法分析模块（Parser）*：
   - 使用 Bison 实现，定义在 `src/parser.y`。
   - 实现 LALR(1) 分析，接受 Pascal-S 完整文法。
   - 在归约动作中调用 `ASTBuilder` 构建 AST 节点。
   - 遇到语法错误后通过恐慌模式恢复（丢弃 Token 直到遇到分号）。
   - 输出 `ProgramNode*` 根节点供后续阶段消费。

3. *语义分析模块（Semantic Annotator）*：
   - 定义在 `src/semantic_annotator.cpp`，是最大的源文件（约 1250 行）。
   - 递归遍历 AST，为每个节点注解类型信息和符号表引用。
   - 实现完整的类型检查、作用域检查、参数验证。
   - 支持编译期常量表达式求值（用于数组边界检查）。
   - 上下文跟踪：函数结果赋值检测、循环深度跟踪（用于 break 验证）。

4. *代码生成模块（Code Generator）*：
   - 定义在 `src/code_generator.cpp`（约 630 行）和 `src/codegen_utils.cpp`（约 440 行）。
   - 基于 Visitor 模式遍历注解后的 AST。
   - 将代码分为四个缓冲区收集：全局声明、函数原型、函数定义、main 函数体。
   - 最终由 `wrapAsCProgram()` 组装为完整的 C 源文件。

5. *错误处理模块（Error Handler）*：
   - 定义在 `include/error_handler.h` 和 `src/error_handler.cpp`。
   - 提供统一的错误报告接口：`report(line, col, message)`。
   - 存储 `std::vector<CompileError>`，记录所有编译错误。
   - 词法错误通过词法分析器内部独立的错误向量管理。

=== 模块之间的关系

各模块之间通过明确的接口进行解耦协作：

- *词法分析 → 语法分析*：Flex 提供的 `yylex()` 每次返回一个 Token，Bison 通过 Bison 内置的 `yylex()` 接口消费 Token 流。
- *语法分析 → 语义分析*：语法分析结束后输出 `ProgramNode*` 根节点，语义分析器以该节点为入口遍历 AST。
- *语义分析 → 代码生成*：语义分析器为 AST 节点填充 `dataType` 和 `symbolEntry`，代码生成器读取这些注解信息生成代码。
- *所有模块 → 错误处理*：各模块均可通过统一的 `ErrorHandler::report()` 接口报告错误。

模块关系图与前文的总体结构图一致。

== 模块接口

=== 词法分析器接口

词法分析器导出以下 C 函数供外部调用：

```cpp
void lexerResetState();              // 重置词法分析器状态
const char* lexerLastLexeme();       // 返回最后一个词素
const char* lexerLastType();         // 返回最后一个 Token 的类型
const char* lexerLastRule();         // 返回最后一次命中的规则标签
int lexerLastLine();                 // 返回最后一个 Token 的行号
int lexerLastColumn();               // 返回最后一个 Token 的列号
void lexerClearErrors();             // 清空错误列表
int lexerErrorCount();               // 返回词法错误计数
const CompileError* lexerErrorAt(int index);  // 获取指定词法错误
int lexerAllocatedStringCount();     // 返回 strdup 次数
int lexerTokenCount();               // 返回 Token 计数
```

=== 语法分析器接口

```cpp
int yyparse();                       // Bison 生成的分析函数
ProgramNode* getParseResultRoot();   // 返回解析结果的根节点
void resetParseResult();             // 重置解析结果
int getParseErrorCount();            // 返回语法错误计数
```

=== 语义分析器接口

```cpp
void SemanticAnnotator::annotate(ASTNode* root);  // 遍历 AST 执行语义注解
```

=== 代码生成器接口

```cpp
std::string CodeGenerator::generate(ProgramNode* root);  // 从 AST 根节点生成 C 代码
```

=== 辅助工具接口

```cpp
std::string CodegenUtils::mapType(DataType t);                   // 类型映射
std::string CodegenUtils::wrapAsCProgram(globals, prototypes, definitions, mainBody);  // 组装 C 程序
std::string CodegenUtils::emitVarDecl(VarDeclNode*);             // 变量声明生成
std::string CodegenUtils::emitConstDecl(ConstDeclNode*);         // 常量声明生成
std::string CodegenUtils::emitProcPrototype(ProcDeclNode*);      // 过程原型生成
std::string CodegenUtils::emitFuncPrototype(FuncDeclNode*);      // 函数原型生成
std::string CodegenUtils::emitProcDecl(ProcDeclNode*);           // 过程定义生成
std::string CodegenUtils::emitFuncDecl(FuncDeclNode*);           // 函数定义生成
std::string CodegenUtils::emitReadStmt(ASTNode* node);           // read 语句生成
std::string CodegenUtils::emitWriteStmt(ASTNode* node);         // write 语句生成
```

== 用户接口设计

编译器以命令行工具形式运行，支持以下参数：

```cpp
pascc -i <input.pas> [-o output.c] [选项]
选项：
  --lex              仅执行词法分析，输出 Token 列表
  --dump-tokens      词法分析并输出 Token 详情及规则命中信息
  --parse            执行词法和语法分析后停止，输出 AST
  --semantic         执行词法、语法和语义分析后停止
  --dump-annotated-ast  语义分析后输出带注解的 AST
  --help             显示帮助信息
```

若未指定 `-o` 参数，编译器自动将输出文件名设为与输入文件同名但扩展名为 `.c` 的文件。

编译器还支持通过环境变量 `PASCC_EVENT_STREAM=1` 启用结构化 JSON 事件流输出，供 TUI（终端用户界面）等工具实时监控编译进度。日志详尽程度由环境变量 `PASCC_LOG_LEVEL` 控制（支持 `debug`、`info`、`warn`、`error` 级别）。

#pagebreak()

= 详细设计

== 词法分析模块

=== 输入输出

- 输入：Pascal-S 源文件（`.pas`）
- 输出：Token 流（由 `yylex()` 每次返回一个 Token，供 Bison 语法分析器消费）

=== 数据结构

词法分析器使用全局状态变量管理扫描上下文：

```c
int g_line = 1;                  // 当前行号
int g_column = 1;                // 当前列号
int g_token_line = 1;            // 当前 Token 起始行号
int g_token_column = 1;          // 当前 Token 起始列号
int g_comment_depth = 0;         // 嵌套注释深度
bool g_in_comment = false;       // 是否在注释内
bool g_in_directive = false;     // 是否在编译器指令内
std::string g_last_rule;         // 最近一次命中的规则标签
std::string g_last_lexeme;       // 最近一次识别的词素
std::string g_last_type;         // 最近一次识别的 Token 类型
std::vector<CompileError> g_lex_errors;  // 词法错误列表
size_t g_strdup_count = 0;       // strdup 次数统计
int g_token_count = 0;           // Token 计数
```

关键字表 `g_keyword_tokens` 为 `std::unordered_map`，将小写关键字字符串映射到 Bison Token 编号，共 33 个关键字条目，其中包含扩展关键字 `type` 与 `record`。
```cpp
const std::unordered_map<std::string, int> g_keyword_tokens = {
	{"program", PROGRAM}, {"const", CONST}, {"var", VAR}, {"integer", INTEGER},
	{"real", REAL}, {"boolean", BOOLEAN}, {"char", CHAR}, {"array", ARRAY},
	{"of", OF}, {"function", FUNCTION}, {"procedure", PROCEDURE},
	{"begin", KW_BEGIN}, {"end", KW_END}, {"if", IF}, {"then", THEN},
	{"else", ELSE}, {"while", WHILE}, {"do", DO}, {"for", FOR},
	{"to", TO}, {"downto", DOWNTO}, {"break", BREAK}, {"read", READ}, {"write", WRITE},
	{"not", NOT}, {"and", AND}, {"or", OR}, {"div", DIV}, {"mod", MOD},
	{"true", TRUE}, {"false", FALSE},
	{"type", TYPE}, {"record", RECORD}
};
```

#figure(
  image("../Week-8-Mid-term/Huhangbin/Figs/lexer-state-machine.svg", width: 92%),
  caption: [词法分析状态机与观测路径：`INITIAL`、`COMMENT`、`DIRECTIVE` 三类扫描状态协同工作，并通过 `YY_USER_ACTION` 统一维护 Token 起始位置。]
)

=== 关键算法

*标识符与关键字识别*：

词法分析器遇到以字母开头的标识符模式时，首先将词素转为小写，然后查关键字表。若命中则返回对应关键字 Token；否则返回 `IDENTIFIER` Token，词素值为小写形式的标识符。这种"转小写后查表"的设计一次性解决了 Pascal-S 不区分大小写的问题。

#algorithm(
  "标识符与关键字识别",
  (
    "输入: 词素 text, 长度 len",
    "text_lower <- toLower(text, len)",
    "if keyword_table.contains(text_lower) then",
    "    return keyword_table[text_lower]",
    "else",
    "    return IDENTIFIER(text_lower)",
    "end if",
  )
)

*位置跟踪*：

通过 Flex 的 `YY_USER_ACTION` 宏在每个 Token 识别前保存起始位置（行、列），识别结束后通过 `advancePosition()` 更新行、列信息。该宏确保每个 Token 都能获得精确的 `yylloc` 位置信息。

```cpp
#define YY_USER_ACTION \
	do { \
		g_token_line = g_line; \
		g_token_column = g_column; \
		yylloc.first_line = g_token_line; \
		yylloc.first_column = g_token_column; \
		advancePosition(yytext, yyleng); \
		yylloc.last_line = g_line; \
		yylloc.last_column = g_column; \
	} while (0);
```

*注释处理*：

采用 Flex 的 Start Condition 机制处理注释。遇到 `{` 时进入 `COMMENT` 状态，同时 `g_comment_depth` 自增。在 `COMMENT` 状态下再次遇到 `{` 则嵌套深度自增，遇到 `}` 则自减——减至零时退出注释模式返回 `INITIAL` 状态。换行符在 `COMMENT` 状态下仍被追踪以保持行号准确。

注释中的换行符通过 `COMMENT` 状态的 `\n` 规则处理，而行号/列号的更新已在 `YY_USER_ACTION` 中完成。

#algorithm(
  "注释嵌套处理",
  (
    "输入: 当前字符 ch",
    "if ch = '{' then",
    "    g_comment_depth <- g_comment_depth + 1",
    "    if g_comment_depth = 1 then",
    "        begin COMMENT state",
    "    end if",
    "else if ch = '}' then",
    "    if g_comment_depth > 0 then",
    "        g_comment_depth <- g_comment_depth - 1",
    "        if g_comment_depth = 0 then",
    "            begin INITIAL state",
    "        end if",
    "    end if",
    "end if",
  )
)

*数值识别*：

词法分析器支持多种数值格式：
- 十进制整数：纯数字序列
- 带小数点的实数：`digits.digits[exponent]`
- 以 `.` 开头的实数：`.digits[exponent]`
- 科学计数法：`digits e[+-]digits`
- 十六进制数：`0x[0-9A-Fa-f]+` 和 `$[0-9A-Fa-f]+`
- 可通过 `isNumericOverflow()` 检测溢出（使用 `strtod` 函数）
- 对格式错误的数值（如多小数点）报告词法错误

*字符串与字符识别*：

- 字符常量：被单引号包裹的单个字符（含转义）识别为 `CHARACTER`
- 字符串常量：被单或双引号包裹的多个字符识别为 `STRING`
- 对未闭合的引号报告相应错误

*编译器指令*：

遇到 `{$` 时进入 `DIRECTIVE` 状态，收集指令体直到 `}`。通过 `isValidDirective()` 验证指令是否合法（`IFDEF`、`UNDEF`、`DEFINE`、`ENDIF`、`ELSE`）。不合法指令产生词法错误但不影响编译流程。

=== 关键规则片段

以下规则片段直接体现了“先大小写归一化，再区分关键字与标识符”的实现策略：

```cpp
{ID_START}{ID_CONT}* {
	if (static_cast<size_t>(yyleng) > MAX_IDENTIFIER_LENGTH) {
		reportLexError(g_token_line, g_token_column,
			"identifier too long", yytext);
		continue;
	}
	std::string lowered = toLower(yytext, yyleng);
	auto it = g_keyword_tokens.find(lowered);
	if (it != g_keyword_tokens.end()) {
		setTokenMeta("Keyword", "keyword_identifier");
		yylval.text = duplicateText(yytext);
		return it->second;
	}
	setTokenMeta("Identifier", "identifier");
	yylval.text = duplicateText(lowered.c_str());
	return IDENTIFIER;
}
```

=== 错误处理

词法分析器可检测并报告以下错误类型：

- 非法字符（任何不匹配上述规则的字符）
- 未闭合的注释（文件结束时仍在 `COMMENT` 状态）
- 未闭合的编译器指令（文件结束时仍在 `DIRECTIVE` 状态）
- 未闭合的字符或字符串字面量
- 数值溢出
- 标识符过长（超过 255 字符）
- 字符串过长（超过 1024 字符）
- 格式错误的数值

所有词法错误记录在 `g_lex_errors` 向量中，通过 `lexerErrorCount()` 和 `lexerErrorAt(int)` 供外部访问。遇到错误时不终止分析，继续处理后续词素。

=== BOM 处理

在 `INITIAL` 状态下，如果是文件第一行第一列遇到 UTF-8 BOM（`\xEF\xBB\xBF`），则静默忽略；否则报告为非法字符。

=== 文件结束处理

当 Flex 遇到 `<<EOF>>` 时，检查是否有未闭合的注释或指令。如有则记录错误，清空状态，并返回 0（表示 Token 流结束）。

== 语法分析模块

=== 输入输出

- 输入：由 `yylex()` 提供的 Token 流
- 输出：`ProgramNode*`（AST 根节点）

=== 数据结构

语法分析器使用以下全局变量：

```c
static ASTBuilder g_astBuilder;      // AST 节点构建器
static ProgramNode* g_parseRoot;     // 解析结果根节点
static int g_rule_line;              // 当前归约行号
static int g_rule_column;            // 当前归约列号
static int g_parse_error_count;      // 语法错误计数
```

Bison 使用 `%union { void* node; char* text; }` 联合体表示文法符号的语义值。AST 节点以 `void*` 形式在文法规则间传递，通过 `asNode()` / `asList()` 辅助函数转换。

所有 Bison Token 分为四类：

- *关键字 Token*：`PROGRAM`、`CONST`、`VAR`、`TYPE`、`RECORD`、`INTEGER`、`REAL`、`BOOLEAN`、`CHAR` 等共 33 个。
- *字面量 Token*：`NUMBER`、`CHARACTER`、`STRING`、`IDENTIFIER`，携带 `text` 属性。
- *运算符 Token*：`ASSIGN`、`LE`、`GE`、`NE`、`DOTDOT`（多字符运算符），以及单字符运算符（`+`、`-` 等，返回 ASCII 值）。
- *分隔符 Token*：`(`、`)`、`[`、`]`、`;`、`,`、`.`、`:` 等。

=== 关键算法

*AST 构建*：

在每个文法产生式的归约动作中，调用 `g_astBuilder` 的工厂方法创建 AST 节点。例如：

```c
// 赋值语句：variable ASSIGN expression
$$ = static_cast
    <void*>(g_astBuilder.makeAssignStmt(asNode($1), asNode($3), currentPos()));
```

`currentPos()` 返回当前归约规则的起始位置（`SourcePos`），确保每个 AST 节点都有正确的位置信息。

*多维数组处理*：

`buildArrayTypeFromRanges()` 函数将 `period` 列表（逗号分隔的多个 `range`）转化为嵌套的 `ArrayTypeNode`。遍历时从最后一个维度向前构建，每层包裹前一层的元素类型。

例如 `array[1..3, 2..4] of integer` 构建为：
```cpp
ArrayType(lower=2, upper=4, elem=ArrayType(lower=1, upper=3, elem=IdentifierNode("integer")))
```

#algorithm(
  "多维数组类型构建 buildArrayTypeFromRanges",
  (
    "输入: rangesNode (period 列表), elemType (元素类型), pos (位置)",
    "if rangesNode = nullptr or rangesNode is not ListNode then",
    "    return elemType",
    "end if",
    "current <- elemType",
    "for i <- rangesNode.children.size() - 1 downto 0 do",
    "    seg <- rangesNode.children[i] as ArrayTypeNode",
    "    current <- makeArrayType(seg.lower, seg.upper, current, pos)",
    "end for",
    "return current",
  )
)

*混合变量访问处理*：

`buildArrayAccessFromIndices()` 函数处理 `variable → id id_varpart` 中的访问链。`id_varpart` 可包含：
- `[ expression_list ]`：数组下标访问（一个或多个索引表达式）。
- `. IDENTIFIER`：字段访问（点号 + 字段名）。

字段名在 `id_varpart` 中通过创建 `FieldAccessNode(base=nullptr, fieldName)` 作为标记节点，与数组索引区分。此标记节点随后在 `buildArrayAccessFromIndices` 中被转换为正确的 `FieldAccessNode`。

#algorithm(
  "混合变量访问解析 buildArrayAccessFromIndices",
  (
    "输入: base (基础标识符), indicesNode (访问链列表 ListNode), pos (位置)",
    "if indicesNode = nullptr or indicesNode is not ListNode then",
    "    return base",
    "end if",
    "current <- base",
    "for each child in indicesNode.children do",
    "    if child is FieldAccessNode (base = nullptr) then",
    "        current <- makeFieldAccess(current, child.fieldName, pos)",
    "    else",
    "        current <- makeArrayAccess(current, child, pos)",
    "    end if",
    "end for",
    "return current",
  )
)

*运算符优先级*：

语法分析器使用 Bison 的 `%left`/`%right`/`%nonassoc` 声明定义运算符优先级和结合性：

```cpp
%right ASSIGN          // 赋值（右结合，优先级最高）
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left OR               // 逻辑或
%left AND              // 逻辑与
%left '=' NE '<' '>' LE GE  // 关系比较
%left '+' '-'          // 加减
%left '*' '/' DIV MOD  // 乘除
%right NOT             // 逻辑非
%right UMINUS          // 一元负号
%right UPLUS           // 一元正号
```

*悬挂 else 消歧*：

使用 `%prec LOWER_THAN_ELSE` 解决经典的"悬挂 else"歧义问题。产生式 `IF expression THEN statement %prec LOWER_THAN_ELSE` 的优先级低于 `ELSE`，从而在遇到 `else` 时选择移入（将 `else` 与最近的 `if` 匹配），而非归约。

*Bison 位置传播*：

通过自定义 `YYLLOC_DEFAULT` 宏实现归约时位置的正确传播：

```c
#define YYLLOC_DEFAULT(Current, Rhs, N)
  (Current).first_line = YYRHSLOC(Rhs, 1).first_line;
  (Current).first_column = YYRHSLOC(Rhs, 1).first_column;
  (Current).last_line = YYRHSLOC(Rhs, N).last_line;
  (Current).last_column = YYRHSLOC(Rhs, N).last_column;
```

该宏使得归约产生式的起始位置为第一个符号的起始位置，结束位置为最后一个符号的结束位置，并同步更新 `g_rule_line`/`g_rule_column` 供 `currentPos()` 使用。

=== 错误处理

语法分析器实现恐慌模式（panic mode）错误恢复，在 `statement_list` 产生式中专门增加错误恢复规则：

```c
| statement_list ';' error
  {
      yyerrok;    // 恢复为正常错误状态
      yyclearin;  // 清除预读 Token
      $$ = $1;    // 继续使用当前语句列表
  }
```

当在语句列表（分号后）遇到语法错误时，Bison 丢弃后续 Token 直到遇到分号，然后通过 `yyerrok` 恢复正常状态，继续分析后续语句。这种设计确保一个语句的语法错误不会导致整个程序的解析失败。

错误信息通过 `yyerror()` 函数输出，格式为：
```
Parse error at 行:列 near 'lexeme': 错误描述
```

=== 语法规则覆盖

语法分析器实现的文法产生式覆盖：

1. *程序结构*：`program → PROGRAM IDENTIFIER opt_program_input ';' program_body '.'`
2. *常量声明*：`const_declarations → CONST const_declaration_list ';'`
3. *类型声明*（扩展）：`type_declarations → TYPE type_declaration_list ';'`，支持 `record` 类型定义
4. *变量声明*：`var_declarations → VAR var_declaration_list ';'`，类型支持基本类型、数组和用户自定义类型
5. *子程序声明*：`subprogram → PROCEDURE/FUNCTION IDENTIFIER formal_parameter ...`，参数支持 `var` 和值参数
6. *语句*：赋值、过程调用、复合语句、条件语句、`for`/`while` 循环、`break`、`read`/`write`
7. *表达式*：支持算术、关系、逻辑运算符，函数调用，数组访问，字段访问

== AST 模块

=== 概述

AST（抽象语法树）模块是编译器所有后续阶段共享的核心数据结构。AST 节点定义在 `include/ast.h` 中，构建器实现在 `src/ast.cpp` 中。整个 AST 体系基于面向对象的多态设计，基类 `ASTNode` 定义了所有节点共有的属性和接口。

=== 数据类型

`DataType` 枚举定义在 `include/common.h` 中：

```c
enum class DataType {
    Integer,    // 整型
    Real,       // 实型
    Boolean,    // 布尔型
    Char,       // 字符型
    Procedure,  // 过程
    Function,   // 函数
    Record,     // record 类型（扩展）
    Unknown     // 未知（初始值）
};
```

=== 节点位置

`SourcePos` 结构记录节点在源码中的位置：

```c
struct SourcePos {
    int line = 0;  // 行号
    int col = 0;   // 列号
};
```

该信息在语法分析归约阶段由 `currentPos()` 提供，确保错误报告能精确定位到源码位置。

=== 核心节点详解

*ProgramNode*：程序根节点，包含 `name`（程序名）和 `children`（子节点为 BlockNode，可选的程序头标识符列表）。该节点在整个 AST 中只有一个实例。

*BlockNode*：作用域块节点。程序体（program_body）的 BlockNode 包含 5 个子节点：`[consts, types, vars, subprograms, compound]`；子程序体（subprogram_body）的 BlockNode 包含 3 个子节点：`[consts, vars, compound]`。

*VarDeclNode*：变量声明节点，`children[0]` 为标识符列表（ListNode），`children[1]` 为类型节点。一个变量声明可同时声明多个同类型变量（如 `a, b, c : integer`）。

*ConstDeclNode*：常量声明节点，`children[0]` 为标识符，`children[1]` 为常量值（可以是 `LiteralNode` 或 `UnaryExprNode`，如负号表达式）。

*ProcDeclNode / FuncDeclNode*：过程/函数声明节点，包含名称、参数列表（`children[0]`）和子程序体（`children[1]`）。`FuncDeclNode` 额外包含 `retType` 字段记录返回值类型。

*ProcCallNode*：过程/函数调用节点，包含名称（`name`）、实参列表（`children`）、`isVarParam` 标识列表（标记哪些实参需按引用传递）、`builtinKind`（标记内置过程 `read`/`write`）。

*ForStmtNode*：for 循环语句节点，包含 `isDownto` 标志（区分 `to` 和 `downto`），`children` 为 `[id, init, end, body]`。

*BinaryExprNode / UnaryExprNode*：表达式节点，通过 `op` 字符串存储运算符。支持的二元运算符：`+`、`-`、`*`、`/`、`div`、`mod`、`and`、`or`、`=`、`<>`、`<`、`>`、`<=`、`>=`。支持的一元运算符：`-`、`not`。

*IdentifierNode*：标识符引用节点，包含 `identifier` 字符串（已转小写）、`isLValue` 标记（是否可作为左值）、`isFunctionResultTarget` 标记（是否是函数结果赋值的左侧）。

*LiteralNode*：字面量节点，包含 `value` 字符串（字面量文本，如 `"3.14"`、`"'a'"`）、`isStringLikeLiteral` 标记（类字符串常量，用于 `write` 语句的 `%s` 格式符）。

*ArrayAccessNode*：数组访问节点，`children[0]` 为基础表达式（数组变量），`children[1]` 为下标表达式。多维数组通过嵌套 ArrayAccessNode 表示。代码生成时利用 `symbolEntry` 中的 `arrayBounds` 计算下标偏移。

*ArrayTypeNode*：数组类型节点，`children[0]` 为下界字面量，`children[1]` 为上界字面量，`children[2]` 为元素类型节点。多维数组通过嵌套 ArrayTypeNode 表示（每层包裹内层类型）。

*CompoundStmtNode*：复合语句节点（对应于 `begin ... end`），`children` 为语句列表。

*ListNode*：通用列表容器节点。不同于其他节点，`ListNode` 的 `children` 动态添加。`ListKind` 字段指示列表语义：

#styled-parameter-table(
  columns: (25%, 75%),
  [ListKind], [用途],
  [Unknown], [未指定],
  [Identifiers], [标识符列表（idlist：逗号分隔的标识符）],
  [Statements], [语句列表（statement_list）],
  [Expressions], [表达式列表（expression_list，含 variable_list）],
  [Parameters], [参数列表（parameter_list）],
  [Declarations], [声明列表（常量声明、变量声明、子程序声明）],
  [ArrayRanges], [多维数组的 period 列表（逗号分隔的 range）],
  [FieldAccess], [字段访问链标记（用于区分数组下标和字段访问）],
)

*AssignStmtNode*：赋值语句节点，`children[0]` 为左值（变量/字段访问），`children[1]` 为右值表达式。支持普通赋值和函数结果赋值两种语义，后者通过左值的 `isFunctionResultTarget` 标记区分。

*IfStmtNode*：条件语句节点，`children[0]` 为条件表达式，`children[1]` 为 then 分支，`children[2]` 为可选的 else 分支（可为 `nullptr`）。

*WhileStmtNode*：while 循环语句节点，`children[0]` 为循环条件，`children[1]` 为循环体语句。

*BreakStmtNode*：break 语句节点，无子节点。在语义分析阶段通过 `loopDepth_` 计数器验证其必须出现在循环体内。

*ParamDeclNode*：参数声明节点，`children[0]` 为标识符列表，`children[1]` 为类型节点，`isVar` 字段标记是否为 `var` 引用参数。

*TypeDeclNode*：类型声明节点（`type name = type_definition;`），`name` 存储类型名，`children[0]` 指向类型定义节点（如 `RecordTypeNode` 或基本类型标识符）。

*RecordTypeNode*：record 类型定义节点，`children[0]` 指向字段列表（ListNode），每个字段为 `FieldDeclNode`。

*FieldDeclNode*：字段声明节点，`children[0]` 为标识符列表（单个字段名），`children[1]` 为类型节点（基本类型）。

*FieldAccessNode*：字段访问节点（`base.fieldName`），`children[0]` 为基础表达式，`fieldName` 存储字段名。语义分析阶段设置为可作赋值左值，代码生成阶段翻译为 C 的点运算符。

=== Arena 内存管理

`ASTBuilder` 类内部维护 `std::vector<std::unique_ptr<ASTNode>> arena_`，所有通过 `create<T>(...)` 模板方法创建的节点均存储在 Arena 中。这种设计优势：

- 节点生命周期与 ASTBuilder 或 AST 根节点一致，无需手动释放。
- 节点指针在 Arena 生命周期内保持稳定。
- 支持 `resetParseResult()` 销毁整个 AST（创建新的 ASTBuilder 实例，旧 Arena 自动析构）。

=== Visitor 模式

`ASTVisitor` 是纯虚基类，为 25 种节点类型各定义了一个 `visit()` 方法。这使编译器能够在不修改 AST 节点类的情况下添加新的遍历逻辑。

`SemanticAnnotator` 和 `CodeGenerator` 分别实现该接口，前者负责语义注解，后者负责代码生成。两者互不干扰，实现了关注点分离。

== 符号表模块

=== 输入输出

- 输入：通过 `insert()` 方法接收符号条目，通过 `lookup()` 方法查询。
- 输出：符号查询结果（`const SymbolEntry*`），插入成功/失败的布尔值。

=== 数据结构

符号表采用 `std::vector<std::unordered_map<std::string, const SymbolEntry*>>` 作为底层容器。每个 `unordered_map` 代表一个作用域，键为归一化（小写）的符号名，值为指向 `SymbolEntry` 的指针。

符号名称归一化通过 `normalizeName()` 静态方法实现，将所有字符转换为小写：

```cpp
static std::string normalizeName(const std::string& name) {
    std::string normalized;
    for (char ch : name) {
        normalized += std::tolower(static_cast<unsigned char>(ch));
    }
    return normalized;
}
```

符号条目 `SymbolEntry` 的字段类型 `SymbolKind` 枚举：

```cpp
enum class SymbolKind {
    Variable,    // 变量
    Constant,    // 常量
    Procedure,   // 过程
    Function,    // 函数
    Parameter,   // 参数
    TypeAlias    // 用户自定义类型别名
};
```

参数/字段信息 `ParamInfo` 结构：

```cpp
struct ParamInfo {
    std::string name;
    DataType type = DataType::Unknown;
    bool isVarParam = false;
};
```

数组边界 `ArrayBound` 结构：

```cpp
struct ArrayBound {
    int lower = 0;
    int upper = -1;
};
```

=== 关键算法

*插入操作（insert）*

将符号插入当前作用域。首先调用 `normalizeName()` 归一化名称，然后检查当前作用域是否已存在同名符号。若存在则返回 `false`，表示重复定义。否则将条目指针存入当前作用域的哈希表中。

条目本身存储于 `entryArena_` 中，确保指针稳定性：

```cpp
bool SymbolTable::insert(SymbolEntry entry) {
    std::string key = normalizeName(entry.name);
    auto& current = scopes_.back();
    if (current.find(key) != current.end()) {
        return false; // 当前作用域中重复定义
    }
    auto unique = std::make_unique<SymbolEntry>(std::move(entry));
    const SymbolEntry* raw = unique.get();
    entryArena_.push_back(std::move(unique));
    current[key] = raw;
    return true;
}
```

*查找操作（lookup）*

从最内层作用域向最外层方向查找。`lookup()` 从 `scopes_` 的末尾向前遍历（对应从局部到全局），在每个作用域的哈希表中查找归一化后的名称。找到则返回 `SymbolEntry*`，找不到返回 `nullptr`。

```cpp
const SymbolEntry* SymbolTable::lookup(const std::string& name) const {
    std::string key = normalizeName(name);
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(key);
        if (found != it->end()) {
            return found->second;
        }
    }
    return nullptr;
}
```

`lookupCurrentScope()` 仅查当前最内层作用域，用于重复定义检测。

#algorithm(
  "符号表查找 lookup",
  (
    "输入: name (原始符号名)",
    "key <- normalizeName(name)  // 统一转小写",
    "for scope in reverse(scopes) from innermost to outermost do",
    "    if scope.contains(key) then",
    "        return scope[key]",
    "    end if",
    "end for",
    "return nullptr  // 未找到",
  )
)

*作用域管理*

`enterScope()`：向 `scopes_` 尾部追加一个空 `unordered_map`，表示进入新作用域。由于过程/函数不可嵌套定义，最大作用域深度为 2（全局 + 局部）。

`exitScope()`：弹出 `scopes_` 尾部元素。注意条目对象保留在 `entryArena_` 中，因此退出作用域后已注册的符号指针仍然有效。

=== 内置符号注册

在语义分析开始前，编译器的 `semantic_register::preregisterBuiltins(symbolTable)` 函数注册以下内置符号：

- `read`：内置过程，参数个数可变、类型可变。
- `write`：内置过程，参数个数可变、类型可变。
- `true` 和 `false`：布尔常量，由 `SemanticAnnotator` 的构造函数注册。

== 语义分析模块

=== 输入输出

- 输入：语法分析阶段输出的 `ASTNode*`（AST 根节点）。
- 输出：注解后的 AST——每个节点填充 `dataType` 和 `symbolEntry`，同时通过 `ErrorHandler` 报告语义错误。

=== 数据结构

语义分析器维护以下上下文状态：

```cpp
SymbolTable& symbolTable_;             // 符号表引用
ErrorHandler& errorHandler_;           // 错误处理器引用
std::vector<std::string> functionContextStack_;  // 当前函数上下文栈
int loopDepth_ = 0;                    // 当前循环嵌套深度
int valueContextDepth_ = 0;            // 值上下文深度
```

- `functionContextStack_`：函数名栈，用于检测函数结果赋值是否出现在正确的函数体中。
- `loopDepth_`：记录循环嵌套深度，`break` 语句验证（必须 `>0`）。
- `valueContextDepth_`：标记当前表达式的求值上下文。当表达式作为右值时（如赋值语句右值、参数等），通过 `annotateValueNode()` 推进；作为左值时则不推进。

=== 关键算法

*注解调度（annotateNode）*

`annotateNode()` 是语义分析的核心分发器，根据节点的 `nodeType` 派发到对应的 `annotate*` 方法：

```cpp
void SemanticAnnotator::annotateNode(ASTNode* node) {
    switch (node->nodeType) {
    case NodeType::Program:      annotateProgram(...);  break;
    case NodeType::Block:        annotateBlock(...);    break;
    case NodeType::VarDecl:      annotateVarDecl(...);  break;
    case NodeType::ConstDecl:    annotateConstDecl(...); break;
    case NodeType::ProcDecl:     annotateProcDecl(...); break;
    case NodeType::FuncDecl:     annotateFuncDecl(...); break;
    // ... 所有 25 种节点类型
    }
}
```

*类型推导（inferType / inferTypeFromTypeNode）*

`inferType(node)` 根据字面量内容或已查询的符号条目前向推导节点类型：

- `true`/`false` → `Boolean`
- 单字符常数（如 `'a'`） → `Char`
- 含小数点或科学计数法的数字 → `Real`
- 纯整数 → `Integer`
- 已绑定 `symbolEntry` 的标识符 → `symbolEntry->type`

`inferTypeFromTypeNode(node)` 专门用于类型声明节点（出现在 `: type` 中），首先检查基本的 `IdentifierNode`（如 `"integer"`），若非基本类型则查询符号表查找用户自定义类型（如 record 类型）。

#algorithm(
  "类型推导 inferType",
  (
    "输入: node (AST 节点)",
    "if node is LiteralNode then",
    "    if node.value = \"true\" or node.value = \"false\" then return Boolean",
    "    if node.value is character literal then return Char",
    "    if node.value contains '.' or 'e' then return Real",
    "    else return Integer",
    "end if",
    "if node is IdentifierNode then",
    "    entry <- symbolTable.lookup(node.identifier)",
    "    if entry != nullptr then",
    "        node.symbolEntry <- entry",
    "        return entry.type",
    "    else report \"undeclared identifier\"",
    "end if",
    "return Unknown",
  )
)

*声明处理*

- `annotateConstDecl`：注解常量值表达式，调用 `inferType()` 推导类型，创建 `SymbolEntry::makeConstant()` 插入符号表，记录常量字面量文本。
- `annotateVarDecl`：从类型节点推导变量类型，收集数组边界信息。处理标识符列表中的每个标识符，依次注册。支持用户自定义类型（record）——通过查找符号表中的 `TypeAlias` 获取 `typeName` 和 `fields`。
- `annotateParamDecl`：类似变量声明处理，同时处理 `var` 参数标记。
- `annotateProcDecl`：将自身名称注册为过程符号，收集参数信息（`SymbolEntry.params`），`enterScope()` 后注解子程序体，最后 `exitScope()`。
- `annotateFuncDecl`：类似过程声明，额外记录返回类型。注解子程序体前将函数名推入 `functionContextStack_`。

*表达式类型检查*

`annotateBinaryExpr` 根据运算符类型对操作数实施不同的类型约束：

#styled-parameter-table(
  columns: (20%, 35%, 45%),
  [运算符], [操作数约束], [结果类型],
  [\+ - \*], [操作数为 numeric（integer/real）], [同操作数类型，除法返回 Real],
  [div mod], [操作数均为 integer], [Integer],
  [and or], [操作数均为 boolean], [Boolean],
  [\= <> < > <= >=], [操作数类型一致], [Boolean],
)

`annotateUnaryExpr`：`not` 要求操作数为 `Boolean`，一元 `-` 要求操作数为 `numeric`。

*标识符注解*

`annotateIdentifier` 查询符号表，若符号不存在则报告"未声明标识符"错误。查询成功后，将节点的 `symbolEntry`、`dataType` 设置为查询结果，并根据符号种类设置 `isLValue`（变量和参数为 `true`，常量为 `false`）。

特殊处理：若当前处于值上下文中且符号是函数，将 `isFunctionResultTarget` 设为 `true`，代码生成时将生成函数调用表达式。

*数组访问检查*

`annotateArrayAccess` 验证下标表达式为 `integer` 类型，基础表达式为数组类型。利用 `tryEvalIntConst()` 进行编译期常量求值，检查下标是否在 `[lower, upper]` 范围内。为支持后续 record 字段访问，将 `symbolEntry` 传递给 `ArrayAccessNode`。

*过程/函数调用检查*

`checkCallArguments()` 验证过程/函数调用的参数数量和类型：

- `read`：所有参数必须为左值（需要存储读入的值）。
- `write`：任意表达式均可。
- 用户自定义过程/函数：参数个数匹配，类型兼容（值参数需 `isAssignable`，`var` 参数需 `isLValue` 且类型完全匹配）。

*赋值语句检查*

`annotateAssignStmt` 处理两种赋值：
1. 常规赋值：左侧必须是可赋值的（左值或字段访问），类型兼容检查（允许 `integer→real` 隐式转换）。
2. 函数结果赋值：左侧标识符的 `isFunctionResultTarget` 为 `true`，且函数名与当前函数上下文栈顶一致。

*循环上下文与 break 检查*

`annotateWhileStmt`、`annotateForStmt` 进入前 `loopDepth_++`，退出后 `loopDepth_--`。`annotateBreakStmt` 验证 `loopDepth_ > 0`，若不在循环内则报告错误。

*常量表达式求值*

`tryEvalIntConst()` 尝试在编译期计算整型常量表达式的值，支持：
- 字面量（整数、字符）
- 一元运算（`+`、`-`）
- 二元运算（`+`、`-`、`*`、`div`、`mod`）

该函数主要用于数组边界检查——当数组下标或边界为常量时，在编译期即可检测越界错误，提供精确的错误消息。

#algorithm(
  "编译期常量求值 tryEvalIntConst",
  (
    "输入: node (AST 节点), result (输出参数)",
    "if node is LiteralNode then",
    "    if node is integer literal then",
    "        result <- strToInt(node.value), return true",
    "    else if node is character literal then",
    "        result <- charToInt(node.value), return true",
    "    end if",
    "end if",
    "if node is UnaryExprNode then",
    "    if node.op = '-' and tryEvalIntConst(node.child, value) then",
    "        result <- -value, return true",
    "    if node.op = '+' and tryEvalIntConst(node.child, value) then",
    "        result <- value, return true",
    "end if",
    "if node is BinaryExprNode then",
    "    if tryEvalIntConst(node.lhs, lv) and tryEvalIntConst(node.rhs, rv) then",
    "        result <- evaluate(lv, node.op, rv), return true",
    "end if",
    "return false  // 非常量表达式",
  )
)

=== 错误处理

语义分析器通过 `ErrorHandler` 报告以下语义错误：

- 未声明的标识符
- 重复定义的标识符（同一作用域内）
- 类型不匹配（赋值、表达式、参数传递）
- 数组下标非整数类型
- 下标越界（编译期可检测的）
- 对非数组类型使用下标访问
- 对非 record 类型使用字段访问
- 不存在的字段名
- `break` 出现在循环之外
- 函数结果赋值不匹配
- 参数个数不匹配
- `read` 的参数不是左值
- `var` 参数要求实参为左值

遇到错误时不中断分析，继续遍历 AST 以收集更多错误信息。

== 代码生成模块

=== 输入输出

- 输入：语义分析后带注解的 AST（`ProgramNode*`）。
- 输出：完整的 C 语言源程序字符串。

=== 数据结构

代码生成器维护四个代码缓冲区：

```cpp
std::string globalDecls_;    // 全局常量、类型定义（struct）、全局变量
std::string prototypes_;     // 函数/过程前置声明
std::string definitions_;    // 函数/过程定义实现
std::string mainBody_;       // main 函数体
```

每个缓冲区按序产出，最终由 `CodegenUtils::wrapAsCProgram()` 组装：

```c
#include <stdio.h>
[globalDecls_]
[prototypes_]
[definitions_]
int main(void) {
    [mainBody_]
    return 0;
}
```

辅助工具类 `CodegenUtils` 提供静态方法用于生成各种声明和语句，包括类型映射、格式符选择、变量/常量声明、过程/函数声明、`read`/`write` 语句等。

Visitor 访问过程中，用 `currentExpr_`（`std::string`）存储当前节点对应的 C 代码片段，供父节点组合使用。

=== 关键算法

*generate() 主入口*

`CodeGenerator::generate(ProgramNode* root)` 执行以下步骤：

1. 从根节点的 children 中找到 BlockNode。
2. 遍历 BlockNode 的子节点：
   - `children[0]`（consts）：遍历常量声明，调用 `emitConstDecl()` 生成 `const` 定义写入 `globalDecls_`。
   - `children[1]`（types）：遍历类型声明（如 record），生成 `typedef struct` 定义写入 `globalDecls_`。
   - `children[2]`（vars）：遍历变量声明，调用 `emitVarDecl()` 生成全局变量声明写入 `globalDecls_`。
   - `children[3]`（subprograms）：遍历子程序，生成原型写入 `prototypes_`，生成定义写入 `definitions_`。
   - `children[4]`（compound）：生成主程序体写入 `mainBody_`。

*类型映射（mapType）*

```cpp
std::string CodegenUtils::mapType(DataType t) {
    switch (t) {
        case DataType::Integer: return "int";
        case DataType::Real:    return "float";
        case DataType::Boolean: return "int";
        case DataType::Char:    return "char";
        default:                return "int";
    }
}
```

注意：Pascal-S 的 `real` 映射为 C 的 `float`，`boolean` 映射为 `int`（以 0/1 表示真假）。

*变量声明生成（emitVarDecl）*

处理以下情况：
- *基本类型变量*：根据 `dataType` 调用 `mapType()` 生成类型名。
- *Record 类型变量*：使用 `symbolEntry->typeName` 作为类型名（如 `person p;`）。
- *数组变量*：递归收集所有维度（通过遍历嵌套的 `ArrayTypeNode`），计算每维大小（`upper - lower + 1`），生成 `type name[size1][size2]...;`。

*常量声明生成（emitConstDecl）*

生成 `const type name = value;` 格式。特殊处理：
- 字符串常量：`const char* name = "...";`
- 布尔常量：`const int true_reserved = 1; const int false_reserved = 0;`（避免与 C 的 `true`/`false` 宏冲突）
- 字符常量：转换为 C 字符字面量

*过程/函数声明生成*

`emitProcDecl()` 和 `emitFuncDecl()` 生成 C 函数定义。关键处理：

- `var` 参数：生成 `Type *param`（指针类型），函数体内对 `var` 参数的使用自动解引用（`(*param)`）。
- 值参数：直接生成 `Type param`。
- Record 类型参数：var 参数使用 `TypeName *param`，值参数使用 `TypeName param`。
- 函数：在函数体开头声明 `_retval` 局部变量，结束时 `return _retval;`。函数体内对函数名的赋值转换为 `_retval = expr;`。

*过程/函数原型生成*

`emitProcPrototype()` 和 `emitFuncPrototype()` 生成前置声明，格式类似定义但以分号结尾（如 `void proc_name(int, int*);`）。

*read/write 语句生成*

`emitReadStmt()` 处理 `read(varlist)`：根据变量类型选择 `scanf` 格式符（`%d`/`%f`/`%c`），对每个变量加 `&` 取址符。

`emitWriteStmt()` 处理 `write(exprlist)`：根据表达式类型选择 `printf` 格式符（`%d`/`%f`/`%c`/`%s`），依次输出各表达式。表达式的 C 代码由 Visitor 的 `currentExpr_` 提供。

*赋值语句生成（visit(AssignStmtNode\*))*

```cpp
void CodeGenerator::visit(AssignStmtNode* node) {
    // 处理左值：变量 → 标识符，字段访问 → base.field
    // 处理右值：表达式 → C 表达式串
    // 函数结果赋值：转换为 _retval = rhs;
    // 常规赋值：生成 lhs = rhs;
}
```

函数结果赋值判断：左值标识符的 `isFunctionResultTarget` 标记。

*for 循环生成（visit(ForStmtNode\*)）*

根据 `isDownto` 标志生成不同的 C 代码：

- `for to`：`for (int id = init; id <= end; ++id) { body }`
- `for downto`：`for (int id = init; id >= end; --id) { body }`

若循环变量未在符号表中注册（即未提前声明），则自动在循环头中声明 `int id`。

*数组访问生成（visit(ArrayAccessNode\*)）*

```cpp
// 基本数组访问 a[i] → a[(i) - lowerBound]
// 多维数组：递归展开到最内层，按数组深度累积下标维度
currentExpr_ = base + "[" + index + " - " + lowerBound + "]";
```

通过 `arrayAccessDepthForCodegen()` 辅助函数计算多维数组的维度深度，在声明时按深度生成多维数组尺寸。

*表达式生成*

- 二元表达式：递归生成左右操作数，按运算符翻译（`mod→%`，`div→/`，`=`→`==`，`<>→!=`，`and→&&`，`or→||`）。整个表达式用括号包裹以确保优先级正确。
- 一元表达式：`not factor`（有特殊处理），`-factor` 直接翻译为 `(-factor)`。
- 布尔字面量：`true→1`，`false→0`。

*过程调用生成（visit(ProcCallNode\*)）*

- `read`：委托 `CodegenUtils::emitReadStmt()` 处理。
- `write`：委托 `CodegenUtils::emitWriteStmt()` 处理。
- 用户定义调用：根据 `isVarParam` 标记决定实参如何传递——引用参数传 `&arg`，值参数直接传 `arg`。

*标识符生成（visit(IdentifierNode\*)）*

- `true` → `1`，`false` → `0`
- 函数名→生成函数调用表达式 `name()`
- `var` 参数→生成 `(*name)`
- 普通变量/常量引用→直接输出标识符名

*Record 类型代码生成*

- `visit(TypeDeclNode*)`：生成 `typedef struct { ... } TypeName;`
- `visit(FieldDeclNode*)`：在 struct 内部生成字段声明 `field_type field_name;`
- `visit(FieldAccessNode*)`：生成 `base.fieldName`（使用 C 的点运算符）

== 错误处理模块

=== 输入输出

- 输入：通过 `report(line, col, message)` 接收各模块报告的错误。
- 输出：错误列表（`std::vector<CompileError>`），供主程序遍历输出。

=== 数据结构

```cpp
struct CompileError {
    int line;
    int column;
    std::string message;
};

class ErrorHandler {
public:
    void report(int line, int col, const std::string& msg);
    bool hasErrors() const;
    const std::vector<CompileError>& errors() const;
    void clear();
private:
    std::vector<CompileError> errors_;
};
```

=== 错误恢复策略

各阶段错误恢复遵循"记录错误、继续分析"原则：

- *词法分析*：遇到非法字符时跳过，记录错误。词法错误存储于独立的 `g_lex_errors` 向量中。
- *语法分析*：恐慌模式——遇到错误后丢弃 Token 直到分号，通过 `yyerrok` 恢复正常分析状态。
- *语义分析*：遇到语义错误时记录到 `ErrorHandler`，继续遍历 AST 其余部分。
- *代码生成*：理论上不会产生新错误（因为输入已经在之前阶段通过验证）。

各阶段结束后主程序检查错误计数。若有错误则输出所有错误信息并中止编译，不会进入后续阶段。

== Record 类型扩展设计

=== 概述

作为项目的扩展功能，编译器实现了完整的 record 类型支持。该扩展涉及语法分析、AST、语义分析和代码生成四个阶段的修改。

=== 新增 AST 节点

为支持 record 类型，新增了 4 种 AST 节点类型：

1. *TypeDeclNode*：类型声明节点，存储类型名（`name`），`children[0]` 指向类型定义节点（如 `RecordTypeNode`）。
2. *RecordTypeNode*：record 类型定义节点，`children[0]` 指向字段列表（ListNode）。
3. *FieldDeclNode*：字段声明节点，表示 record 内的一个字段，`children[0]` 为标识符列表，`children[1]` 为类型节点。
4. *FieldAccessNode*：字段访问节点，表示 `base.fieldName`，`children[0]` 为基础表达式，`fieldName` 为字段名。

=== 语法分析扩展

在 parser.y 中新增以下文法规则：

```
type_declarations → type type_declaration_list ';'
type_declaration → id '=' type
type → ... | record_type | id   // 扩展：支持 record 和用户自定义类型
record_type → record field_list end
field_list → field_declaration | field_list ';' field_declaration
field_declaration → idlist ':' basic_type

variable → id id_varpart       // id_varpart 扩展：增加字段访问
id_varpart → ... | id_varpart '.' id  // 字段访问
```

其中 `value_parameter` 的语法从 `idlist ':' basic_type` 改为 `idlist ':' type`，以支持 record 类型作为参数。同理，数组元素类型规则 `type` 也扩展为支持 `array [period] of type`，实现 record 数组。

=== 字段访问标记机制

`id_varpart` 在遇到 `. IDENTIFIER` 时创建 `FieldAccessNode(base=nullptr, fieldName)` 作为标记节点。`ListKind` 被切换为 `FieldAccess` 以表明该列表包含混合类型的访问（数组下标和字段名）。

`buildArrayAccessFromIndices()` 函数遍历 `id_varpart` 列表时，通过 `dynamic_cast<FieldAccessNode*>` 检查节点是字段标记还是数组索引，据此构建对应的 `ArrayAccessNode` 或 `FieldAccessNode`。

=== 语义分析扩展

符号表层面：新增 `SymbolKind::TypeAlias` 符号种类，`SymbolEntry` 新增 `fields`（`vector<ParamInfo>`）和 `typeName`（`string`）字段用于存储 record 字段信息和用户类型名。

语义分析实现：

- `annotateTypeDecl()`：处理 `type T = record ... end`，遍历字段列表收集字段信息（包括检测重复字段名），创建 `SymbolEntry::makeTypeAlias()` 注册到符号表。
- `annotateRecordType()`：设置 record 类型节点的 `dataType` 为 `Record`。
- `annotateFieldAccess()`：验证基础表达式为 record 类型，从符号表的 `fields` 中查找字段名，设置结果类型为对应字段的类型。标记为左值（字段访问可作为赋值左值）。
- `annotateVarDecl()` 和 `annotateParamDecl()`：当类型节点为用户自定义类型时，从符号表查找 `TypeAlias`，复制 `typeName` 和 `fields` 到变量/参数的符号条目中。

=== 代码生成扩展

- `visit(TypeDeclNode*)`：生成 `typedef struct { fields... } TypeName;`
- `visit(FieldDeclNode*)`：在 struct 内生成字段声明。
- `visit(FieldAccessNode*)`：生成 `base.fieldName`（C 的点访问语法）。
- `emitVarDecl()`：检测 record 类型变量，使用 `typeName` 作为类型名。
- `emitProcPrototype()`/`emitFuncPrototype()`：处理 record 类型参数，var 参数使用 `TypeName *`，值参数使用 `TypeName`。

=== Record 参数与数组支持

在基础 record 支持之后，进一步扩展了以下功能：

1. *Record 作为参数*：修改 `value_parameter` 和 `var_parameter` 的类型规则，允许 `idlist : type`（而非仅 `basic_type`），使 record 类型可作为过程/函数的参数传递。值参数直接传递 record 值（C 按值传递 struct），`var` 参数传递 record 指针。

2. *Record 数组*：修改 `type` 规则为 `array '[' period ']' of type`（而非仅 `of basic_type`），使数组元素类型可递归定义，支持 `array[1..10] of person` 这样的 record 数组声明。

3. *混合访问*：支持 `people[i].age` 这样的链式访问——先数组下标访问再字段访问，通过 `FieldAccessNode` 标记机制实现语法层的正确解析。

= 源程序清单

== 目录结构

```text
code/
├── include/                     # 头文件
│   ├── ast.h                    # AST 节点定义
│   ├── symbol_table.h           # 符号表接口
│   ├── semantic_annotator.h     # 语义分析器接口
│   ├── code_generator.h         # 代码生成器接口
│   ├── codegen_utils.h          # 代码生成工具接口
│   ├── error_handler.h          # 错误处理接口
│   ├── token.h                  # Token 结构定义
│   ├── common.h                 # 公共类型定义
│   ├── debug_utils.h            # 调试工具接口
│   ├── parser_bridge.h          # 语法分析桥接
│   └── semantic_register.h      # 内置符号注册
├── src/                         # 源文件
│   ├── lexer.l                  # Flex 词法分析
│   ├── parser.y                 # Bison 语法分析
│   ├── main.cpp                 # 程序入口
│   ├── ast.cpp                  # AST 实现
│   ├── symbol_table.cpp         # 符号表实现
│   ├── semantic_annotator.cpp   # 语义分析实现
│   ├── code_generator.cpp       # 代码生成实现
│   ├── codegen_utils.cpp        # 代码生成工具实现
│   ├── error_handler.cpp        # 错误处理实现
│   ├── debug_utils.cpp          # 调试工具实现
│   └── semantic_register.cpp    # 内置符号注册实现
├── test/                        # 测试用例集
│   ├── cases/valid/             # 合法测试用例
│   ├── cases/invalid/           # 非法测试用例
│   ├── close_set/               # 核心测试集
│   ├── open_set/                # 开放测试集
│   ├── semantic_unit/           # 语义分析单元测试
│   └── semantic_stubs/          # 语义分析桩代码
├── docs/                        # 设计文档
├── scripts/                     # 构建与工具脚本
├── Makefile                     # GNU Make 构建
└── CMakeLists.txt               # CMake 构建
```

== 编程风格

项目遵循统一的 C++ 编程规范：

- 使用 C++17 标准。
- 标识符命名：类名使用 PascalCase（如 `SemanticAnnotator`），方法名使用 camelCase（如 `annotateNode`），成员变量以下划线结尾（如 `symbolTable_`），全局变量以 `g_` 前缀（如 `g_comment_depth`）。
- 代码缩进：使用 4 空格缩进。
- 头文件保护：使用 `#ifndef PASCC_*_H` 格式。
- 内存管理：优先使用 RAII 和智能指针（`std::unique_ptr`）。AST 节点和符号条目使用 Arena 模式管理生命周期。
- Visitor 模式：语义分析和代码生成通过 Visitor 模式解耦，各节点通过 `accept()` 方法接受访问。

= 程序测试

== 测试环境

- 操作系统：macOS 26.4 / Windows 11 / Ubuntu 20.04
- 编译器：g++ 14.2.0 / Apple Clang 16.0.0
- 测试框架：Shell 脚本自动化测试
- 测试用例格式：Pascal-S 源文件（`.pas`）

== 测试策略

测试采用分层策略，每层独立验证，上层测试依赖下层通过：

1. *词法分析单元测试*：验证 Token 识别、关键字区分、标识符大小写归一、行列号准确、词法错误检测。
2. *语法分析测试*：验证文法正确接受、AST 结构构建正确、运算符优先级、悬挂 else 消歧、语法错误恢复。
3. *语义分析单元测试*：验证符号表操作、类型检查、作用域管理、参数验证、break 上下文检查。
4. *代码生成测试*：验证生成的 C 代码可编译运行、类型映射正确、数组偏移计算正确、var 参数处理正确。
5. *集成测试*：验证完整的 Pascal-S 程序（如 GCD、Quicksort）可编译并输出正确结果。
6. *错误恢复测试*：验证多错误程序能否正确报告所有错误而不崩溃。

== 测试用例概览

测试用例覆盖词法、语法、语义、代码生成各阶段。合法用例验证编译器正确处理各类 Pascal-S 结构并生成可运行的 C 代码；非法用例验证编译器能准确检测并报告各类错误。

Close set（核心测试集）包含约 200+ 个用例，覆盖基础功能。Open set（开放测试集）包含约 150+ 个用例，覆盖边界情况和复杂场景。此外还有专门的 parser 测试集、语义单元测试集，以及 record 扩展功能的专项测试用例。

== 测试方法

测试采用自动化 Shell 脚本驱动：

- `test/run_tests.sh`：基础冒烟测试。
- `test/run_all_tests.sh`：运行所有测试用例。
- `test/test_lexer_unit.sh`：词法分析单元测试。
- `test/test_semantic_unit.sh`：语义分析单元测试。
- `test/run_parser_tests.sh`：语法分析测试。
- `test/run_closeset_check.sh`：核心测试集（Close set）。
- `test/run_openset_check.sh`：开放测试集（Open set）。

每个测试用例流程：
1. 编译器读取 `.pas` 源文件并生成 `.c` 输出。
2. 对合法用例：生成的 C 代码用 `gcc` 编译，运行并与预期输出比较。
3. 对非法用例：验证编译器输出正确的错误信息和错误位置。

== 关键测试用例

*GCD 函数程序*：编译官方示例 GCD 程序（通过欧几里得算法计算最大公约数），验证生成的 C 代码可正确运行并输出预期结果。

*Quicksort 程序*：完整快速排序实现，验证编译器能够处理较复杂的程序结构、数组操作、过程调用和递归。

*Record 类型测试*：
- `record_basic`：基本 record 声明与字段访问。
- `record_param_value`：record 作为值参数传递。
- `record_param_var`：record 作为 var 参数传递。
- `record_param_multiple`：多个 record 参数的混合传递。
- `record_array_basic`：record 数组的声明与访问。
- `record_array_sum`：对 record 数组元素的聚合计算。

= 课程设计总结

== 项目完成情况

本项目成功实现了一个完整的 Pascal-S 到 C 语言编译器，实现了全部课程设计要求：

- *词法分析*：Flex 实现，完整支持 Pascal-S 词法规范，包括标识符大小写不敏感、嵌套注释、编译器指令、多种数字格式。
- *语法分析*：Bison 实现 LALR(1) 分析，归约时构建 AST，恐慌模式错误恢复，正确处理运算符优先级和悬挂 else。
- *语义分析*：完整的类型检查系统，符号表管理，作用域控制，编译期常量求值，上下文敏感的 break 检查和函数结果赋值检查。
- *代码生成*：基于 Visitor 模式生成可编译运行的 C 代码，正确处理类型映射、数组偏移、var 参数、函数返回值。
- *扩展功能*：实现 record 类型（结构体）的完整支持，包括 record 声明、字段访问、作为参数和数组元素。

总计约 5657 行源代码（含 Flex/Bison 文件），支持 232 个测试用例。

== 每位成员的工作与收获

*王嘉晗（语法分析）*：负责设计和维护 AST 契约，在 Bison 文件中实现归约建树动作。主要收获包括深入理解了 LALR(1) 分析原理、AST 数据结构设计方法，以及如何在语法分析阶段高效构建中间表示。

*张宸宇（组长/语义分析）*：负责符号表设计与实现，语义注解器的完整编写，以及整体架构设计。掌握了符号表作用域管理的实现技巧、类型系统的设计要点，以及编译器中语义检查的广度与深度。

*胡航宾（词法分析）*：负责 `src/lexer.l` 的核心规则实现与维护，包括关键字/标识符识别、数字与字符串字面量规则、嵌套注释与编译器指令状态处理、行列号跟踪，以及词法错误收集接口。通过这部分工作，进一步掌握了 Flex Start Condition、规则优先级、错误恢复式扫描，以及如何为后续 parser/semantic 阶段提供稳定且可观测的 Token 输入。

*李思远（代码生成基础）*：负责代码生成器框架和基础语句（赋值、条件、复合语句）的 Visitor 实现。掌握了 Visitor 模式的实战应用、C 代码生成的关键技术细节，以及如何编写可扩展的代码生成框架。

*谢康（代码生成进阶）*：负责数组访问、过程调用、var 参数传递的代码生成实现。深入理解了 Pascal-S 与 C 在类型系统和参数传递机制上的差异，掌握了数组偏移计算和指针参数转换等关键技术。

== 设计过程中遇到的主要问题及解决方案

1. *标识符大小写处理*：Pascal-S 不区分大小写，但 C 区分。解决方案：在词法分析阶段将标识符统一转为小写返回，符号表 Key 同样转小写。这样后续所有阶段自然获得大小写不敏感的语义。

2. *数组下标偏移*：Pascal 数组可从任意整数开始（如 `A[3..9]`），C 从 0 开始。解决方案：代码生成时自动计算偏移 `a[i - lower_bound]`，其中 `lower_bound` 从语义分析阶段的 `arrayBounds` 信息获取。

3. *var 参数传递*：Pascal 的 `var` 参数是引用传递，C 无直接对应。解决方案：var 参数在函数声明时生成指针类型，调用时传地址 `&arg`，函数体内自动解引用 `*arg`。

4. *函数返回值实现*：Pascal 中函数通过给函数名赋值返回结果（`func_name := value`），C 使用 `return`。解决方案：代码生成引入局部变量 `_retval`，将函数名赋值转换为 `_retval = value`，函数结束时 `return _retval`。

5. *悬挂 else 歧义*：`if a then if b then s1 else s2` 中的 else 归属歧义。解决方案：使用 Bison 的 `%prec LOWER_THAN_ELSE` 优先级声明，让 Bison 自动选择移入（将 else 与最近 if 绑定）。

6. *Record 类型的混合访问解析*：`people[i].age` 既包含数组下标又包含字段访问，需要正确解析。解决方案：在 `id_varpart` 中使用 `FieldAccessNode(base=nullptr)` 作为字段名标记，与数组索引区分。在 `buildArrayAccessFromIndices()` 中通过 `dynamic_cast` 区分处理。

7. *Record 类型信息传播*：record 类型的字段信息需要在编译的不同阶段传递。解决方案：在语义分析阶段将 `TypeAlias` 的 `fields` 和 `typeName` 复制到每个使用该类型的变量的符号条目中。代码生成阶段直接从变量的 `symbolEntry` 读取字段信息。

== 改进建议

1. *增加中间代码优化*：可在 AST 和代码生成之间增加一个中间表示（IR）层，实施常量折叠、死代码消除等优化。

2. *支持更多 Pascal 特性*：可扩展支持变体记录（variant record）、集合类型（set）、文件 I/O 等更高级的 Pascal 特性。

3. *增强错误恢复*：当前语法分析的恐慌模式恢复点仅限分号，可增加对 `end`、`until` 等更多同步记号的支持。

4. *完善测试覆盖*：可增加更多边界值测试、压力测试和模糊测试，进一步提升编译器的健壮性。

5. *支持嵌套 record 字段访问*：当前实现不支持 `a.b.c` 形式的嵌套 record 字段访问（仅支持一级字段访问 `a.b`），可进一步扩展。
