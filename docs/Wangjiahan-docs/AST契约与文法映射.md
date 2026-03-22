# Pascal-S AST 契约与文法映射（对齐当前实现）

## 1. 文档目的
本文件用于统一语法分析（A 角色）与语义/代码生成模块对 AST 的理解。

适配文件：
- `include/ast.h`
- `src/ast.cpp`

## 2. 全局约定

1. AST 所有 `children` 顺序是强约束，不可随实现者改变。
2. 列表统一使用 `ListNode`，并通过 `ListKind` 标注语义。
3. `BlockNode` 与 `ListNode` 职责边界（强约束）：
- `BlockNode` 仅用于承载语法结构块（如 program_body、subprogram_body）。
- `ListNode` 仅用于承载同类可变长序列（如 idlist、statement_list、expression_list、parameter_list、声明集合、数组区间集合）。
4. 任何地方禁止用 `BlockNode` 代替列表，或用 `ListNode` 代替结构块。
5. 空产生式默认处理：
- 列表型非终结符：返回空 `ListNode`
- `else_part`：返回 `nullptr`
6. 过程调用与函数调用统一节点 `ProcCallNode`，区别由语义阶段判定。
7. 数组类型与数组访问严格分离：
- 类型定义：`ArrayTypeNode`
- 访问表达式：`ArrayAccessNode`
8. `SourcePos` 挂载规则：所有 AST 节点必须携带 `SourcePos`，并记录该节点对应语法结构的起始位置；如实现支持结束位置，建议同时记录结束位置。

## 3. ListKind 约定

- `Identifiers`：`idlist`
- `Statements`：`statement_list`
- `Expressions`：`expression_list` / `variable_list`
- `Parameters`：`parameter_list`
- `Declarations`：声明集合（const/var/subprogram）
- `ArrayRanges`：`period` 的维度区间列表

补充约定（落地唯一化）：
- `variable_list` 在当前契约中统一映射为 `ListNode(Expressions)`，用于 `read(...)` 参数传递。
- 若后续希望区分“可读入左值列表”与“普通表达式列表”，应新增 `ListKind`，不要复用现有种类并改变语义。

## 4. 节点契约

## 4.1 ProgramNode
- 字段：`name` = 程序名
- `children`：
- `children[0]`：`BlockNode`（program_body）
- 可选 `children[1]`：`ListNode(Identifiers)`（`program id(idlist)`）

## 4.2 BlockNode
结构容器，建议固定布局：
- `children[0]`：`ListNode(Declarations)` const 声明集
- `children[1]`：`ListNode(Declarations)` var 声明集
- `children[2]`：`ListNode(Declarations)` subprogram 声明集（program_body）
- `children[3]`：`CompoundStmtNode`（主程序体）

子程序体可用：
- `children[0]` const 集
- `children[1]` var 集
- `children[2]` compound

## 4.3 VarDeclNode
- `children[0]`：`ListNode(Identifiers)`
- `children[1]`：类型节点（`IdentifierNode` 或 `ArrayTypeNode`）

## 4.4 ConstDeclNode
- `children[0]`：`IdentifierNode`（常量名）
- `children[1]`：常量值节点（`LiteralNode` 或 `UnaryExprNode`）

## 4.5 ProcDeclNode
- 字段：`name`
- `children[0]`：`ListNode(Parameters)`（元素为 `ParamDeclNode`）
- `children[1]`：`BlockNode`（过程体）

## 4.6 FuncDeclNode
- 字段：`name`、`retType`
- `children[0]`：`ListNode(Parameters)`
- `children[1]`：`BlockNode`（函数体）

## 4.7 ParamDeclNode
- 字段：`isVar`
- `children[0]`：`ListNode(Identifiers)`（形参名列表）
- `children[1]`：类型节点（`IdentifierNode` 或 `ArrayTypeNode`）

## 4.8 AssignStmtNode
- `children[0]`：左值（`IdentifierNode` / `ArrayAccessNode`）
- `children[1]`：右值表达式

## 4.9 IfStmtNode
- `children[0]`：条件
- `children[1]`：then 分支
- `children[2]`：仅在存在 `else` 时填充；无 `else` 时不填充该槽位（当前实现对齐）

## 4.10 ForStmtNode
- 字段：`isDownto`（`false`=to，`true`=downto）
- `children[0]`：循环变量（`IdentifierNode`）
- `children[1]`：初值表达式
- `children[2]`：终值表达式
- `children[3]`：循环体语句

## 4.10.1 WhileStmtNode
- `children[0]`：循环条件表达式
- `children[1]`：循环体语句

## 4.11 CompoundStmtNode
- `children`：语句序列，顺序与源码一致

## 4.12 ProcCallNode
- 字段：`name`
- `children`：实参表达式列表
- `isVarParam`：语义阶段回填，语法阶段默认 `false`

## 4.13 BinaryExprNode
- 字段：`op`
- `children[0]`：lhs
- `children[1]`：rhs

## 4.14 UnaryExprNode
- 字段：`op`（`not` 或 `-`）
- `children[0]`：expr

## 4.15 IdentifierNode
- 字段：`identifier`
- `isLValue`：语义阶段标注

## 4.16 LiteralNode
- 字段：`value`

## 4.17 ArrayAccessNode
- 字段：`lowerBound`
- `children[0]`：base（通常 `IdentifierNode`，多维可嵌套）
- `children[1]`：index

约束：
- `lowerBound` 由语义阶段在符号解析后回填；语法阶段仅建结构，不做下标合法性判定。

## 4.18 ArrayTypeNode
- `children[0]`：lower bound
- `children[1]`：upper bound
- `children[2]`：elemType

多维数组表示：
- 递归嵌套 `ArrayTypeNode` 到 `children[2]`

## 4.19 ListNode
- 字段：`kind`
- `children`：列表元素序列


## 4.20 节点不变量（最小集）

1. `BinaryExprNode.children.size() == 2`。
2. `UnaryExprNode.children.size() == 1`。
3. `AssignStmtNode.children.size() == 2`，且 `children[0]` 必须是左值节点。
4. `ForStmtNode.children.size() == 4`。
5. `ParamDeclNode.children.size() == 2`。
6. `ArrayTypeNode.children.size() == 3`。
7. `ArrayAccessNode.children.size() == 2`。
8. `ProcCallNode.children` 中元素按源码实参顺序排列。
9. `CompoundStmtNode` 可为空（对应 `begin end`）。

## 5. Pascal-S 文法 -> AST 映射

## 5.1 程序结构
- `programstruct -> program_head ; program_body .`
  - `ProgramNode`
  - `children[0] = BlockNode(program_body)`
- `program_head -> program id`
  - 提供 `ProgramNode.name`
- `program_head -> program id ( idlist )`
  - `ProgramNode.name = id`
  - 可选 `children[1] = ListNode(Identifiers)`
- `program_body -> const_declarations var_declarations subprogram_declarations compound_statement`
  - `BlockNode`

## 5.2 声明
- `const_declarations -> ε | const const_declaration ;`
  - `ListNode(Declarations)`
- `const_declaration -> id = const_value`
  - `ConstDeclNode`
- `var_declarations -> ε | var var_declaration ;`
  - `ListNode(Declarations)`
- `var_declaration -> idlist : type`
  - `VarDeclNode`
- `idlist -> id | idlist , id`
  - `ListNode(Identifiers)`

## 5.3 类型
- `basic_type -> integer | real | boolean | char`
  - `IdentifierNode`
- `type -> basic_type`
  - 直接返回基础类型节点
- `type -> array [ period ] of basic_type`
  - `ArrayTypeNode`（多维递归）
- `period -> digits .. digits | period , digits .. digits`
  - `ListNode(ArrayRanges)`

## 5.4 子程序
- `subprogram_declarations -> ε | subprogram_declarations subprogram ;`
  - `ListNode(Declarations)`
- `subprogram -> subprogram_head ; subprogram_body`
  - `ProcDeclNode` / `FuncDeclNode`
- `subprogram_head -> procedure id formal_parameter`
  - `ProcDeclNode(name, paramList, body)`
- `subprogram_head -> function id formal_parameter : basic_type`
  - `FuncDeclNode(name, paramList, retType, body)`
- `formal_parameter -> ε | ( parameter_list )`
  - `ListNode(Parameters)`
- `parameter_list -> parameter | parameter_list ; parameter`
  - `ListNode(Parameters)`
- `parameter -> var_parameter | value_parameter`
  - `ParamDeclNode`
- `var_parameter -> var value_parameter`
  - `ParamDeclNode(isVar=true)`
- `value_parameter -> idlist : basic_type`
  - `ParamDeclNode(isVar=false)`
- `subprogram_body -> const_declarations var_declarations compound_statement`
  - `BlockNode`

## 5.5 语句
- `compound_statement -> begin statement_list end`
  - `CompoundStmtNode`
- `statement_list -> statement | statement_list ; statement`
  - `ListNode(Statements)`（最终装入 `CompoundStmtNode`）
- `statement -> ε`
  - `nullptr`
- `statement -> variable assignop expression`
  - `AssignStmtNode`
- `statement -> func_id assignop expression`
  - `AssignStmtNode`
- `statement -> procedure_call`
  - `ProcCallNode`
- `statement -> compound_statement`
  - 直接返回 `CompoundStmtNode`
- `statement -> if expression then statement else_part`
  - `IfStmtNode`
- `else_part -> ε | else statement`
  - `nullptr` 或语句节点
- `statement -> for id assignop expression to expression do statement`
  - `ForStmtNode`
- `statement -> for id assignop expression downto expression do statement`
  - `ForStmtNode(isDownto=true)`
- `statement -> while expression do statement`
  - `WhileStmtNode`
- `statement -> read ( variable_list )`
  - `ProcCallNode(name="read")`
- `statement -> write ( expression_list )`
  - `ProcCallNode(name="write")`

## 5.6 表达式
- `expression -> simple_expression`
  - 直接返回
- `expression -> simple_expression relop simple_expression`
  - `BinaryExprNode(op=relop)`
- `simple_expression -> term`
  - 直接返回
- `simple_expression -> simple_expression addop term`
  - `BinaryExprNode(op=addop)`
- `term -> factor`
  - 直接返回
- `term -> term mulop factor`
  - `BinaryExprNode(op=mulop)`
- `factor -> num`
  - `LiteralNode`
- `factor -> variable`
  - `IdentifierNode` / `ArrayAccessNode`
- `factor -> ( expression )`
  - 直接返回
- `factor -> id ( expression_list )`
  - `ProcCallNode`
- `factor -> not factor`
  - `UnaryExprNode(op="not")`
- `factor -> uminus factor`
  - `UnaryExprNode(op="-")`

## 5.7 变量与调用
- `variable -> id id_varpart`
  - 简单变量：`IdentifierNode`
  - 数组变量：`ArrayAccessNode`
- `id_varpart -> ε | [ expression_list ]`
  - 空或下标列表
- `procedure_call -> id | id ( expression_list )`
  - `ProcCallNode`
- `expression_list -> expression | expression_list , expression`
  - `ListNode(Expressions)`
- `variable_list -> variable | variable_list , variable`
  - `ListNode(Expressions)`（本契约固定）

## 5.8 `period` 编码约定（数组维度）

- `period` 语法：`digits .. digits | period , digits .. digits`
- AST 编码：`ListNode(ArrayRanges)`，每一维区间编码为一个 `ArrayTypeNode` 片段：
  - `children[0]`：lower bound（`LiteralNode`）
  - `children[1]`：upper bound（`LiteralNode`）
  - `children[2]`：暂置空，待最终拼接到完整 `ArrayTypeNode` 时再连接元素类型
- 推荐实现顺序：先收集所有维度区间到 `ListNode(ArrayRanges)`，再从后向前折叠构造嵌套 `ArrayTypeNode`。

## 6. 运算符映射

- `relop`: `=`, `<>`, `<`, `<=`, `>`, `>=` -> `BinaryExprNode.op`
- `addop`: `+`, `-`, `or` -> `BinaryExprNode.op`
- `mulop`: `*`, `/`, `div`, `mod`, `and` -> `BinaryExprNode.op`
- 一元：`not`、`-` -> `UnaryExprNode.op`

## 7. 阶段职责边界

语法阶段（A 角色）负责：
- 节点类型、children 顺序、`ListKind`、`SourcePos`

语义阶段负责：
- `dataType`、`symbolEntry`、`isLValue`
- `ProcCallNode.isVarParam` 回填
- `ArrayAccessNode.lowerBound` 回填
- 函数返回值规则检查（Pascal-S 常见规则：通过对函数名赋值实现返回）

代码生成阶段负责：
- 读取已注解 AST 生成目标代码

## 7.1 SourcePos 传播规则（Parser 实现约束）

1. 叶子节点（`IdentifierNode`、`LiteralNode`）的 `SourcePos` 取对应 token 的起始位置。
2. 复合节点默认取该产生式首个关键 token 的起始位置（例如 `if`、`for`、`begin`）。
3. 列表节点（`ListNode`）取该列表第一次出现元素的位置；空列表取触发该空产生式的语法位置。
4. 若后续引入结束位置，不改变上述“起始位置”规则。

## 7.2 文本规范（Identifier / Literal）

1. `IdentifierNode.identifier` 使用词法归一化后的文本（当前词法实现为小写）。
2. `LiteralNode.value` 保存词法原始词素文本（例如字符引号、字符串引号、`$FF`、`0x1A` 形式保留）。
3. 若语义阶段需要规范化字面量值，应写入语义字段，不回写 `value` 原文。

## 8. 错误处理约定（语法阶段）

1. 当前实现基线：使用 Bison 错误恢复规则与 `yyerror`，在可恢复场景下继续解析。
2. `statement -> ε` 返回 `nullptr`，表示合法空语句，不视为错误节点。
3. 若后续引入 `ErrorNode`，建议仅作为扩展方案：
- 语法错误位置用 `ErrorNode` 占位，避免整棵 AST 中断。
- `ErrorNode` 至少携带：错误消息、`SourcePos`、原产生式上下文。
- 语义阶段统一收集并上报 `ErrorNode`。

## 9. AST 核心示例（课设高频）

示例 1：`var a,b:integer;`

```plaintext
VarDeclNode {
  children[0] = ListNode(Identifiers) {
    children = [IdentifierNode("a"), IdentifierNode("b")]
  },
  children[1] = IdentifierNode("integer")
}
```

示例 2：`for i := 1 to 10 do begin a := i + 1 end;`

```plaintext
ForStmtNode {
  children[0] = IdentifierNode("i"),
  children[1] = LiteralNode(1),
  children[2] = LiteralNode(10),
  children[3] = CompoundStmtNode {
    children = [
      AssignStmtNode {
        children[0] = IdentifierNode("a"),
        children[1] = BinaryExprNode {
          op = "+",
          children = [IdentifierNode("i"), LiteralNode(1)]
        }
      }
    ]
  }
}
```

## 10. 边界场景清单

1. `begin end` 必须生成 `CompoundStmtNode`，且 `children` 允许为空。
2. `ProcCallNode` 无参数时，`children.size() == 0`。
3. `IfStmtNode` 无 `else` 时，不填充 `children[2]`（与当前 `ast.cpp` 实现一致）。
4. 多维数组访问（如 `a[i,j]`）需按约定拆解为嵌套 `ArrayAccessNode` 或等价结构，项目内必须唯一化实现。
5. 函数返回值语义不在语法阶段判定，语法阶段仅保证结构正确。
6. `ForStmtNode.isDownto` 必须与源代码方向保持一致（`to=false`，`downto=true`）。

## 11. 建议扩展（不影响当前实现对齐）

1. 为 `LiteralNode` 增加 `LiteralType`（`Integer/Real/Boolean/Char`），减少仅靠字符串值带来的歧义。
2. 引入 `ErrorNode` 并在恢复解析中挂载，提升错误聚合能力。
3. 若需更严格位置诊断，可把 `SourcePos` 从“起始位置”扩展为“起止区间”。

## 12. 实现一致性检查清单（Parser / Visitor / Memory）

以下清单用于语法建树联调时的快速自检，避免“契约正确但实现跑不通”。

### 12.1 Parser 一致性

1. Bison 终结符命名必须与 lexer 返回值一致，禁止在 parser 侧私自改名。
2. 所有 `IDENTIFIER` / `NUMBER` / `CHARACTER` / `STRING` 的 `yylval.text` 在语义动作消费后必须释放，避免内存泄漏。
3. 列表型非终结符必须统一返回 `ListNode`，并正确设置 `ListKind`。
4. `IfStmtNode` 的无 `else` 场景采用唯一实现：不填充 `children[2]`。
5. 关键 AST 节点（声明、语句、表达式）应在语义动作中挂载 `SourcePos` 起始位置。

### 12.2 Lexer-Parser token 对齐（最小表）

| 词法返回 | parser token | 用途 |
| --- | --- | --- |
| `program` | `PROGRAM` | 程序头 |
| 标识符 | `IDENTIFIER` | 名称引用 |
| 数字常量 | `NUMBER` | 字面量 |
| 字符常量 | `CHARACTER` | 字面量 |
| 字符串常量 | `STRING` | 字面量（扩展） |
| `:=` | `ASSIGN` | 赋值 |
| `<=` | `LE` | 关系运算 |
| `>=` | `GE` | 关系运算 |
| `<>` | `NE` | 关系运算 |
| `..` | `DOTDOT` | 数组区间 |
| `begin` | `KW_BEGIN` | 复合语句起始 |
| `end` | `KW_END` | 复合语句结束 |

### 12.3 词法语义值内存所有权

1. `yylval.text` 的释放责任在 parser 动作（消费方），遵循“谁消费谁释放”。
2. parser 未消费文本值的 token，不应在 lexer 侧分配 `yylval.text`。
3. 关键字 token 默认不携带文本语义值；如调试需要携带，必须在 parser 增加统一释放策略（例如 `%destructor`）。

### 12.4 Visitor 一致性

1. 每次扩展 `ASTVisitor` 接口后，所有继承类（语义、代码生成、调试打印）必须同步补全 `visit(...)`。
2. 如下游暂未支持某新节点，必须提供可编译的占位 `visit(...)`，禁止留下纯虚函数未实现状态。
3. 下游读取 `children` 时必须严格按本契约的顺序约定，不允许按“节点类型猜测”读取。

### 12.5 内存与生命周期一致性

1. AST 节点统一由 `ASTBuilder` 的 `arena_` 管理生命周期，外部不得手动 `delete` AST 节点。
2. 语法动作中构造 AST 时，禁止把临时栈对象地址写入 `children`。
3. lexer 分配的字符串与 AST arena 生命周期相互独立：
- 词法字符串由 parser 动作负责释放。
- AST 节点对象由 `ASTBuilder` 统一托管。

### 12.6 联调最小通过标准

1. 词法通过：能稳定输出与 parser 声明一致的 token。
2. 建树通过：`yyparse` 成功后可获得非空根节点。
3. 语义通过：不因访问器缺失而编译失败。
4. 生成通过：代码生成阶段至少可遍历 AST 并输出可编译骨架代码。

---

如果实现与本契约冲突，以本契约为准，先修正 parser 归约动作，再修正下游语义/代码生成读取逻辑。
