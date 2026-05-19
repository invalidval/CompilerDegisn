== 代码生成模块

=== 输入输出与总体架构

代码生成模块的输入是语义分析后带类型注解和符号表绑定的 AST（`ProgramNode*`），输出是完整的 C 语言源程序字符串。

代码生成器 `CodeGenerator` 采用 Visitor 模式遍历 AST。每个 `visit` 方法处理一种 AST 节点，将生成的 C 代码片段写入 `currentExpr_`（`std::string`），供父节点组合使用。`CodegenUtils` 提供一组静态工具方法用于生成声明、读写语句和最终组装。

=== 四区代码缓冲

代码生成器为不同性质的声明将代码分流到四个缓冲区：

```cpp
std::string globalDecls_;   // 全局常量 (const)、类型定义 (typedef struct)、全局变量
std::string prototypes_;    // 函数/过程的前置声明
std::string definitions_;   // 函数/过程的完整定义（含函数体）
std::string mainBody_;      // main 函数体中的语句
```

`generate()` 入口方法手动解析 `BlockNode` 的子节点而非递归遍历，将每个子节点类别精确路由到正确的缓冲区：

```cpp
std::string CodeGenerator::generate(ProgramNode* root) {
    reset();
    // 程序 (program) → 包含一个 BlockNode
    auto* block = dynamic_cast<BlockNode*>(root->children[0]);

    // children[0]: 常量声明 → 全局声明区
    for (auto* declNode : block->children[0]->children)
        globalDecls_ += CodegenUtils::emitConstDecl(declNode);

    // children[1]: 类型声明 (record 等) → 全局声明区
    for (auto* typeNode : block->children[1]->children)
        globalDecls_ += emitNode(typeNode);

    // children[2]: 变量声明 → 全局声明区
    for (auto* varNode : block->children[2]->children)
        globalDecls_ += CodegenUtils::emitVarDecl(varNode);

    // children[3]: 子程序 → 原型区 + 定义区
    for (auto* subNode : block->children[3]->children) {
        if (auto* proc = dynamic_cast<ProcDeclNode*>(subNode)) {
            prototypes_   += CodegenUtils::emitProcPrototype(proc);
            definitions_  += CodegenUtils::emitProcDecl(proc, *this);
        } else if (auto* func = dynamic_cast<FuncDeclNode*>(subNode)) {
            prototypes_   += CodegenUtils::emitFuncPrototype(func);
            definitions_  += CodegenUtils::emitFuncDecl(func, *this);
        }
    }

    // children[4]: 主程序体 → main 函数体
    visit(block->children[4]);
    mainBody_ = currentExpr_;

    return CodegenUtils::wrapAsCProgram(globalDecls_, prototypes_,
                                        definitions_, mainBody_);
}
```

四个缓冲区依次拼接，最终由 `wrapAsCProgram()` 组装为完整 C 文件：

```c
#include <stdio.h>
[globalDecls_]        // const int N = 100;
                      // typedef struct { ... } Person;
                      // int a, b;
[prototypes_]         // void swap(int *x, int *y);
                      // int gcd(int a, int b);
[definitions_]        // void swap(int *x, int *y) { ... }
                      // int gcd(int a, int b) { ... }
int main(void) {
    [mainBody_]
    return 0;
}
```

=== 类型映射

Pascal-S 基本类型到 C 类型的一一映射由 `mapType()` 完成：

```cpp
DataType::Integer → "int"
DataType::Real    → "float"    // Pascal real 映射为 C float
DataType::Boolean → "int"      // boolean 使用 0/1 表示
DataType::Char    → "char"
default           → "int"
```

读写语句中的格式符选择同样基于类型：

```cpp
DataType::Integer → "%d"
DataType::Real    → "%f"
DataType::Boolean → "%d"
DataType::Char    → "%c"
```

=== 变量声明生成

`emitVarDecl()` 处理三种变量声明场景：

*基本类型：* 从 `VarDeclNode` 中提取变量名列表和类型节点，生成 `int a, b, c;` 形式的声明。

*数组类型：* 遍历嵌套的 `ArrayTypeNode` 链，逐维收集下界和上界（从 `ArrayTypeNode` 的字面量子节点中读取），计算每维大小 `upper - lower + 1`，生成 `type name[N1][N2]...;`。例如，Pascal 声明 `arr: array[1..10, 2..8] of integer` 生成 C 声明 `int arr[10][7];`。

```cpp
// 遍历数组类型链，收集每一维的大小
while (typeNode->nodeType == NodeType::ArrayType) {
    int lower = std::stoi(asLiteral(typeNode->children[0])->value);
    int upper = std::stoi(asLiteral(typeNode->children[1])->value);
    dimensions.push_back(upper - lower + 1);
    typeNode = typeNode->children[2];  // 进入下一维或元素类型
}
// 生成: type name[dims[0]][dims[1]]...;
```

*Record 类型：* 从符号表条目获取用户定义的类型名（如 `person`），直接使用该名称 — 不参与 `mapType()` 映射。

=== 常量声明生成

`emitConstDecl()` 生成 `const type name = value;`。特殊处理包括布尔常量（`true→1`, `false→0`）、负数常量（识别 `UnaryExprNode` 包裹的字面量，输出 `-<value>`），以及字符串类常量（使用 `const char*` 并通过 `pascalCharLiteralToCString()` 将 Pascal 单引号字符常量转换为 C 双引号字符串）。常量命名时自动避免与 C 保留字冲突（如 `true_reserved`、`false_reserved`）。

=== 过程与函数翻译

==== 原型声明

原型生成分为过程原型和函数原型，二者的区别在于返回类型：

```cpp
// 过程原型
void procedureName(int param1, int *param2);
// 函数原型
int functionName(int param, float *varParam);
```

参数列表的生成需处理 Pascal 的多变量声明语法：`var a, b, c: integer` 允许在一个参数组中声明多个同类型变量。遍历时对每个变量名单独生成参数条目。`var` 参数在类型前插入 `*`（指针）。

==== 过程定义

`emitProcDecl()` 生成完整的 `void name(params) { body }` 函数定义。函数体的生成通过调用者传入的 `CodeGenerator` 引用：`cg.visit(body)` 遍历过程体的 `BlockNode`，将局部常量、局部变量和语句依次生成为缩进后的 C 代码。

==== 函数定义与 \_retval 机制

`emitFuncDecl()` 是函数翻译的核心。Pascal 语法中函数通过“对函数名赋值”来设置返回值（如 `gcd := a`），而 C 通过 `return` 语句返回。为此，引入一个名为 `_retval` 的局部变量作为中间桥梁：

```cpp
int funcName(int param) {
    int _retval;              // 1. 声明同类型的返回值临时变量
    // ...
    _retval = expr;            // 2. 对函数名的赋值转化为对 _retval 的赋值
    // ...
    return _retval;            // 3. 函数末尾返回 _retval
}
```

`_retval` 机制涉及三个环节的配合：

- *声明*：`emitFuncDecl()` 在函数体开头插入 `int _retval;`（类型由函数返回类型决定）。
- *赋值*：当 `AssignStmtNode` 检测到左值为一个函数标识符（`symbolEntry->kind == SymbolKind::Function`）时，生成 `_retval = <rhs>;` 而非 `funcName = <rhs>;`。
- *返回*：`emitFuncDecl()` 在函数体末尾追加 `return _retval;`。
- *read 读入*：当 `emitReadStmt()` 检测到实参标记 `isFunctionResultTarget` 时，生成 `&_retval` 作为 scanf 的目标地址，使 `read(f)` 能够将输入值直接写入函数返回值。

=== var 引用参数的指针翻译

Pascal 的 `var` 参数表示引用传递，在 C 中通过指针实现。这一翻译涉及三个方面：

*声明侧——参数类型加 `*`：* 过程原型 `procedure swap(var x, y: integer)` 生成 C 原型 `void swap(int *x, int *y);`。

*使用侧——自动解引用：* 在 `IdentifierNode` 的 visit 中，检查符号表条目的 `isVarParam` 标记。若为 var 参数，在标识符外包裹 `(*)`：

```cpp
void CodeGenerator::visit(IdentifierNode* node) {
    if (node->symbolEntry->kind == SymbolKind::Parameter
        && node->symbolEntry->isVarParam) {
        currentExpr_ = "(*" + node->identifier + ")";
        return;
    }
    currentExpr_ = node->identifier;
}
```

因此，Pascal 过程体中的 `x := x + 1`（`x` 为 var 参数）生成 C 代码 `(*x) = (*x) + 1;`。

*调用侧——自动取址：* 在 `ProcCallNode` 的 visit 中，检查参数位置的 `isVarParam[i]` 标记。若对应形参为 var，则在实参前添加 `&`：

```cpp
if (node->isVarParam[i]) {
    currentExpr_ += "&" + argExpr;
} else {
    currentExpr_ += argExpr;
}
```

调用 `swap(a, b)` 生成 `swap(&a, &b);`。

*避免双重取址：* 当一个 var 参数本身作为另一个 var 参数传入时，它已经是指针，不应再加 `&`。代码通过检查实参 `IdentifierNode` 的 `isVarParam` 属性来识别此情况，直接传递变量名。

=== 数组访问的下标偏移

Pascal 数组可以从任意整数起始（如 `array[3..9] of integer`），而 C 数组总是从 0 开始。`visit(ArrayAccessNode*)` 必须为每次数组访问生成下标偏移：

```cpp
void CodeGenerator::visit(ArrayAccessNode* node) {
    std::string base = emitNode(node->children[0]);   // 数组名或上层 ArrayAccess
    std::string index = emitNode(node->children[1]);  // 下标表达式

    // 从符号表获取当前维度的下界
    int lowerBound = node->lowerBound;
    if (node->symbolEntry && node->symbolEntry->isArray) {
        int depth = arrayAccessDepthForCodegen(node);
        lowerBound = node->symbolEntry->arrayBounds[depth].lower;
    }

    if (lowerBound == 0) {
        currentExpr_ = base + "[" + index + "]";       // 无需偏移
    } else {
        currentExpr_ = base + "[(" + index + ") - "
                     + std::to_string(lowerBound) + "]"; // 偏移
    }
}
```

辅助函数 `arrayAccessDepthForCodegen()` 递归计数当前 `ArrayAccessNode` 的嵌套层次，以从符号表的多维边界数组中选择正确的下界。例如，对于 `matrix[2, 3]`（声明为 `array[1..3, 1..3]`），外层访问深度为 0（下界 1），内层访问深度为 1（下界 1），生成 `matrix[(2) - 1][(3) - 1]`。

=== 表达式生成

==== 二元表达式

二元运算符通过直接查表转换，整个表达式包裹在括号内以保证优先级：

- 算术：`+` → `+`，`-` → `-`，`*` → `*`，`/` → `/`
- 整数运算：`div` → `/`，`mod` → `%`
- 关系比较：`=` → `==`，`<>` → `!=`，`<`/`<=`/`>`/`>=` 保持不变
- 逻辑运算：`and` → `&&`，`or` → `||`

生成方式为递归生成左右操作数，插入对应 C 运算符，再整体加括号：

```cpp
currentExpr_ = "(" + emitNode(node->children[0])
             + " " + op + " " + emitNode(node->children[1]) + ")";
```

==== 一元表达式

`not` 运算符的处理区分操作数类型——对整数使用位非 `~`，对布尔使用逻辑非 `!`。一元负号 `-` 直接翻译为 `(-operand)`。结果同样用括号包裹。

==== 字面量与标识符

字面量中，布尔常量 `true`/`false` 映射为 `1`/`0`，其余（整数、实数、字符）直接输出。标识符中，除前述的 var 参数解引用外，还处理：零参数函数的引用——当 Pascal 代码中直接使用函数名（如 `write(factorial)`），生成 C 的函数调用表达式 `factorial()`。

=== 控制流语句

==== 赋值语句

赋值语句的生成区分函数返回值赋值和普通赋值：

```cpp
void CodeGenerator::visit(AssignStmtNode* node) {
    std::string lhs = emitNode(node->children[0]);
    std::string rhs = emitNode(node->children[1]);

    // 函数结果赋值：f := expr → _retval = expr;
    if (auto* idNode = dynamic_cast<IdentifierNode*>(node->children[0]))
        if (idNode->symbolEntry->kind == SymbolKind::Function)
            lhs = "_retval";

    currentExpr_ = lhs + " = " + rhs + ";";
}
```

==== if-then-else 语句

生成标准 C 语法 `if (cond) { thenPart } else { elsePart }`。条件表达式来自递归生成（如前所述，已带括号），then 和 else 分支各自用花括号包裹。

==== while 语句

生成 `while (cond) { body }`。条件表达式的生成与 if 相同。

==== for 循环

for 循环根据 `to`/`downto` 方向生成不同的比较和步进操作：

```cpp
void CodeGenerator::visit(ForStmtNode* node) {
    std::string cmp  = node->isDownto ? ">=" : "<=";
    std::string step = node->isDownto ? "--"  : "++";

    currentExpr_ = "for (" + loopVar + " = " + initValue
                 + "; " + loopVar + " " + cmp + " " + endValue
                 + "; " + loopVar + step + ") { " + body + " }";
}
```

因此 `for i := 1 to 10 do` 生成 `for (i = 1; i <= 10; i++)`，而 `for i := 10 downto 1 do` 生成 `for (i = 10; i >= 1; i--)`。若循环变量未在符号表中注册（即未提前声明），则自动在 for 头部生成 `int i` 声明。

==== break 与复合语句

`break` 直接输出 `break`。复合语句（`begin...end`）顺序遍历各子语句并连接，对过程调用和 break 两种节点自动补充分号（这些节点的 visit 方法不输出尾部分号，由复合语句节点统一处理）：

```cpp
void CodeGenerator::visit(CompoundStmtNode* node) {
    std::string result;
    for (auto* stmt : node->children) {
        result += emitNode(stmt);
        if (needsTrailingSemicolon(stmt))
            result += ";";
        result += "\n";
    }
    currentExpr_ = result;
}
```

=== read/write 内建过程的翻译

`read(varlist)` 通过 `emitReadStmt()` 翻译为 C 的 `scanf()`。对每个参数根据其类型选择格式符，并在变量名前加 `&` 取址。特殊处理包括：对 var 参数（已是指针，不加 `&`）和函数返回值目标（使用 `&_retval`）。

```cpp
// Pascal: read(a, b);        // a: integer, b: real
// C:      scanf("%d%f", &a, &b);
```

`write(exprlist)` 通过 `emitWriteStmt()` 翻译为 `printf()`。同样根据类型选择格式符，但不加 `&`（输出值而非写入地址）。对字符串类常量使用 `%s` 格式。

```cpp
// Pascal: write(x, 'hello'); // x: integer
// C:      printf("%d%s", x, "hello");
```

=== Record 类型的代码生成

Record 类型的翻译涉及三个 AST 节点：

*TypeDeclNode*（类型定义）：生成 `typedef struct { ... } TypeName;`。遍历 record 体内的字段列表，逐一生成字段声明。

*FieldDeclNode*（字段声明）：处理 `id1, id2: fieldType` 的 Pascal 多变量声明语法，为每个标识符生成 `ctype fieldName;`。

*FieldAccessNode*（字段访问）：Pascal 的 `r.fieldName` 直接翻译为 C 的 `r.fieldName`（使用 `.` 运算符）。基对象（`r`）通过递归生成来处理数组元素 `arr[i]` 或更深层的嵌套。

=== 代码生成辅助函数

除主类外，代码生成模块还提供若干文件级辅助函数：

- `toLowerCopy()`：字符串转小写，用于 `true`/`false` 的不区分大小写识别。
- `indentText()`：对多行代码字符串每行增加指定数量的前导空格，用于函数体内的缩进格式。
- `arrayAccessDepthForCodegen()`：递归计算 `ArrayAccessNode` 链的嵌套深度，用于多维数组的维度下界查找。
- `needsTrailingSemicolon()`：判断特定节点类型（`ProcCall`、`BreakStmt`）是否需要补充分号。
