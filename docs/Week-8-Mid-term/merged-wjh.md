---
marp: true
theme: default
size: 16:9
paginate: true
header: "编译原理课程设计-中期汇报"
# footer: "张宸宇 胡航宾 王嘉晗 李思远 谢康  ·  2026.4.22"
math: katex
---

<!-- #region 样式 -->
<style>
:root {
  --bg: #ffffff;
  --fg: #1a2a44;
  --blue: #0b63d0;
  --blue-dark: #073b8c;
  --blue-light: #f0f7ff;
  --accent: #002d72; /* 深蓝装饰色 */
  --header-footer-color: #8899aa;
  --code-font-size: 19px;
}

section::before {
  content: "";
  position: absolute;
  top: 20px;
  right: 30px;
  width: 160px;
  height: 160px;
  background-image: url('Figs/bupt-logo-small.png');
  background-size: contain;
  background-repeat: no-repeat;
  z-index: 10;
}

section {
  font-size: 22px;
}
h1 { font-size: 40px; color: #1a237e; }
h2 { font-size: 28px; color: #283593; border-bottom: 2px solid #3949ab; padding-bottom: 0.2em; }

/* 双栏布局 */
.cols {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1.5em;
  align-items: start;
}
.cols-6040 { display: grid; grid-template-columns: 3fr 2fr; gap: 1.5em; align-items: start; }
.cols-4060 { display: grid; grid-template-columns: 2fr 3fr; gap: 1.5em; align-items: start; }

/* 代码字号 */
pre, code {
  font-size: var(--code-font-size);
  line-height: 1.35;
}

/* 提示框 */
.box-note { background:#e8f4fd; border-left:4px solid #2196F3; padding:0.6em 1em; border-radius:4px; margin:0.4em 0; }
.box-tip  { background:#e8f5e9; border-left:4px solid #4CAF50; padding:0.6em 1em; border-radius:4px; margin:0.4em 0; }

/* 小字注释 */
.note { font-size:14px; color:#888; margin-top:auto; padding-top:0.4em; border-top:1px solid #e0e0e0; }


/* 过渡页样式 */
section.transition {
  background-color: var(--bg);
  justify-content: center;
}
section.transition h1 {
  font-size: 120px;
  color: var(--blue);
  opacity: 0.2;
  position: absolute;
  right: 50px;
  bottom: 20px;
  border: none;
}
section.transition h2 {
  font-size: 50px;
  border-left: 10px solid var(--accent);
  padding-left: 30px;
  border-bottom: none; 
}

/* 目录样式 */
section.toc
{
  
  font-size: 28px;
  color: var(--fg);
  padding: 2em;
}

section.toc h2
{
  
  font-size: 46px;
  color: var(--fg);
}

/* 文本块 */
.text-block {
  background: var(--blue-light);
  border-top: 5px solid var(--accent);
  padding: 1em;
  border-radius: 4px;
}

/* 紧凑页（解决单页内容溢出） */
section.compact {
  font-size: 20px;
}
section.compact li {
  margin: 0.15em 0;
}
section.compact table {
  font-size: 18px;
}

/* 关键机制页：进一步压缩行距与间距，避免左下角溢出 */
section.mechanism-tight {
  font-size: 20px;
}
section.mechanism-tight li {
  margin: 0.1em 0;
}
section.mechanism-tight .box-tip {
  margin-top: 0.25em;
  padding: 0.5em 0.8em;
}

</style>
<!-- #endregion -->

<!-- _paginate: false -->
<!-- _header: "" -->
<!-- _footer: "" -->

# Pascc 中期汇报

## 第八周-详细设计汇报

张宸宇 胡航宾 王嘉晗 李思远 谢康 · 2026.4.22

---
<!-- _class: toc -->
## 目录

1. [词法分析]()
2. [语法分析]()
3. [符号表和语义分析]()
4. [代码生成]()
5. [错误处理与恢复]()

---

<!-- #region 每个人负责自己的部分 -->
<!-- _class: transition -->
# 02
## 语法分析与建树
### 汇报人：王嘉晗

---

<!-- _class: compact -->
## 我的模块职责与交付

<div class="cols">
<div>

**负责范围（Parser）**

- 基于 Bison 实现 Pascal-S 语法归约
- 在归约动作中构建 AST，不在语法阶段生成 C 代码
- 输出 `ProgramNode*` 根节点供语义/生成阶段使用
- 提供语法错误定位与恢复能力

**核心交付文件**

- `include/ast.h`：AST 节点、枚举与建树契约定义。
- `include/parser_bridge.h`：解析结果导出与状态重置接口。
- `src/ast.cpp`：AST 节点和 `ASTBuilder` 具体实现。
- `src/parser.y`：文法规则、归约建树、错误恢复主实现。


<div class="box-tip">

本次汇报以当前代码实现为准，设计文档已同步更新。

</div>

</div>
<div>

**交付能力清单**

| 能力 | 当前状态 |
|---|---|
| 程序结构/声明解析 | 已完成 |
| 语句与表达式优先级 | 已完成 |
| AST 建树映射 | 已完成 |
| 错误恢复 | 已完成 |
| 冲突治理（RR） | 已完成 |

</div>
</div>

---

<!-- _class: compact -->
## 语法模块架构与接口设计

<div class="cols-4060">
<div>

**模块内部结构**

- 入口：`yyparse()` 驱动归约
- 建树：`ASTBuilder` 在每条关键归约动作中创建节点
- 状态：`g_parseRoot`、`g_parse_error_count` 维护解析结果
- 输出：`ProgramNode*` 供后续阶段统一消费

</div>
<div>

```cpp
// parser.y（内部状态 + 入口）
static ASTBuilder g_astBuilder;
static ProgramNode* g_parseRoot = nullptr;
static int g_parse_error_count = 0;

int yyparse(void);

ProgramNode* getParseResultRoot() {
  return g_parseRoot;
}
```

</div>
</div>

---

<!-- _class: compact -->
## 对外桥接接口（parser_bridge）

<div class="cols">
<div>

- `getParseResultRoot()`：导出 AST 根
- `resetParseResult()`：重置解析状态
- `getAstBuilder()`：暴露建树器给联调流程
- `getParseErrorCount()`：统一失败判定依据
- `yyparse()`：统一语法解析入口
- `yyin`：输入流句柄

<div class="box-tip">

接口层提供统一访问入口，外部模块通过桥接接口而非直接访问解析器内部变量。

</div>

</div>
<div>

```cpp
ProgramNode* getParseResultRoot();
void resetParseResult();
ASTBuilder& getAstBuilder();
int getParseErrorCount();

int yyparse(void);
extern FILE* yyin;
```

</div>
</div>

---

## 文法详细设计（规则分层）

<div class="cols">
<div>

**主线规则**

1. 程序结构：`program -> PROGRAM IDENTIFIER opt_program_input ';' program_body '.'`
2. 声明结构：`const/var/subprogram`
3. 语句结构：赋值、调用、复合、分支、循环、I/O、break
4. 表达式结构：关系层 > 加法层 > 乘法层 > 因子层

**AST 映射策略**

- 声明：`ConstDeclNode` / `VarDeclNode` / `ProcDeclNode` / `FuncDeclNode`
- 语句：`AssignStmtNode` / `IfStmtNode` / `ForStmtNode` / `WhileStmtNode`
- 表达式：`BinaryExprNode` / `UnaryExprNode`
- 数组：`ArrayTypeNode` / `ArrayAccessNode`

</div>
<div>

```yacc
expression:
    simple_expression
  | simple_expression '=' simple_expression
  | simple_expression NE simple_expression

simple_expression:
    term
  | simple_expression '+' term
  | simple_expression '-' term
  | simple_expression OR term

term:
    factor
  | term '*' factor
  | term DIV factor
  | term AND factor
```

<div class="box-note">

说明：这里按当前 `parser.y` 实现展开，不抽象化为伪文法。

</div>

</div>
</div>

---

<!-- _class: compact mechanism-tight -->
## 关键机制详细设计

<div class="cols-4060">
<div>

**二义性处理（dangling else）**

- `%nonassoc LOWER_THAN_ELSE`
- `%nonassoc ELSE`
- `IF ... THEN statement %prec LOWER_THAN_ELSE`

**错误恢复**

- `statement_list ';' error` 
- 恢复动作中配合 `yyerrok`、`yyclearin`
- 避免单错误触发连续报错

**位置系统**

- 启用 `%locations`
- `YYLLOC_DEFAULT` 同步归约位置
- `yyerror` 输出 `line:column + near lexeme`

</div>
<div>

```yacc
statement_list:
  nonempty_statement
| statement_list ';' statement
| statement_list ';' error

nonempty_statement:
  IF expression THEN statement ELSE statement
| IF expression THEN statement %prec LOWER_THAN_ELSE
```

<div class="box-tip">

目标：可恢复、可定位，同时保持建树语义稳定。

</div>

</div>
</div>

---

<!-- _class: compact -->
## 详细设计：声明与类型子系统

<div class="cols-4060">
<div>

**设计目标**

- 将程序体固定为 4 槽位：const / var / subprogram / compound
- 声明区统一归并为 `ListNode(Declarations)`
- 数组类型通过区间列表折叠为嵌套 `ArrayTypeNode`

**关键结构约定**

- `program_body` 必定生成 `BlockNode`
- `const_declaration_list`、`var_declaration_list` 统一追加模式
- `period/range` 先收集区间，再由 `buildArrayTypeFromRanges` 生成类型树

</div>
<div>

```yacc
program_body:
  const_declarations
  var_declarations
  subprogram_declarations
  compound_statement

var_declaration:
  idlist ':' type

type:
    basic_type
  | ARRAY '[' period ']' OF basic_type

period:
    range
  | period ',' range
```

<div class="box-note">

该子系统的核心是“声明归并 + 类型树构造”，保证语义阶段可直接消费结构化类型信息。

</div>

</div>
</div>

---

<!-- _class: compact -->
## 详细设计：语句与表达式子系统

<div class="cols-4060">
<div>

**语句层设计**

- `statement` 提供空语句能力，`statement_list_opt` 负责复合语句中的可选语句列表
- `nonempty_statement` 承载所有可建树语句分支
- `statement_list` 采用“非空起步 + 追加归约”

**表达式层设计**

- 四层结构：`expression / simple_expression / term / factor`
- 运算符优先级由分层和 `%left/%right` 共同保证
- `factor` 同时覆盖字面量、变量、调用、一元运算

</div>
<div>

```yacc
statement_list:
  nonempty_statement
| statement_list ';' statement
| statement_list ';' error

expression:
    simple_expression
  | simple_expression '=' simple_expression

simple_expression:
    term
  | simple_expression '+' term

term:
    factor
  | term '*' factor
```

<div class="box-tip">

该子系统保证“可恢复、可扩展、可建树”：错误语句可跳过，主干语法仍可持续归约。

</div>

</div>
</div>

---

## 详细设计落地情况

**已完成内容**

1. 语法规则分层与优先级设计落地到 Bison 文法
2. AST 建树动作与节点契约已对齐
3. 语法错误恢复与位置定位机制已实现
4. 语法兼容性补齐项（空参/空实参/UPLUS/链式下标）已并入主线
5. RR 冲突完成等价治理，解析逻辑与对外接口保持稳定
6. 对外桥接接口已支撑语义/代码生成阶段联调

**设计验证方式**

```bash
bash scripts/build.sh
bash test/run_parser_tests.sh
bash test/run_closeset_check.sh
bash test/run_openset_check.sh
```

<div class="note">注：本页强调“详细设计如何落地到实现”，测试结果用于证明设计有效性。</div>


<!--- #endregion -->