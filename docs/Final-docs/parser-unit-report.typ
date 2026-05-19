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

#align(center)[
  #title("目  录")
]
#outline(
  title: none
)

#pagebreak()

#align(center)[
  #title("语法分析单元测试报告")
]


= 语法分析测试

所有语法测试均使用以下命令执行（`--parse` 模式仅做词法和语法分析，输出 AST 结构后停止）：

```bash
./build/pascc -i <file.pas> --parse
```

合法用例预期输出 `Parse succeeded.` 并打印 AST 树形结构；非法用例预期输出 `Parse error` 及错误位置信息。

== 合法用例（parser_valid/，共 8 个）

=== pv01_minimal

最小合法程序，验证解析器对空程序体的处理。

*源码：*
```pascal
program pv01;
begin
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    List
    List
    List
    List
    CompoundStmt
```

所有的 `List` 节点（常量声明、类型声明、变量声明、子程序声明列表）均为空，`CompoundStmt` 为空的 begin-end 语句块。AST 结构完整，层级正确。

=== pv02_assign

变量声明与基本赋值语句。

*源码：*
```pascal
program pv02;
var a: integer;
begin
  a := 1
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    List
    List
    List
      VarDecl
        List
          Identifier (a)
        Identifier (integer)
    List
    CompoundStmt
      AssignStmt
        Identifier (a)
        Literal
```

`VarDecl` 节点包含变量名列表（`Identifier (a)`）和类型标识（`integer`）。`AssignStmt` 节点的左值为 `Identifier`，右值为字面量 `Literal`。

=== pv03_if_else

if-then-else 条件语句，验证悬挂 else 消歧。

*源码：*
```pascal
program pv03;
var a: integer;
begin
  if a = 0 then
    a := 1
  else
    a := 2
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    ...
    CompoundStmt
      IfStmt
        BinaryExpr
          Identifier (a)
          Literal
        AssignStmt
          Identifier (a)
          Literal
        AssignStmt
          Identifier (a)
          Literal
```

`IfStmt` 节点包含三个子节点：条件表达式 `BinaryExpr (a = 0)`、then 分支赋值、else 分支赋值。else 正确绑定到最近的 if。

=== pv04_while

while-do 循环语句。

*源码：*
```pascal
program pv04;
var a: integer;
begin
  while a < 10 do
    a := a + 1
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    ...
    CompoundStmt
      WhileStmt
        BinaryExpr
          Identifier (a)
          Literal
        AssignStmt
          Identifier (a)
          BinaryExpr
            Identifier (a)
            Literal
```

`WhileStmt` 包含条件表达式 `BinaryExpr (a < 10)` 和循环体 `AssignStmt`。循环体内的二元表达式 `BinaryExpr (a + 1)` 嵌套正确。

=== pv05_for_to

for-to 递增循环语句。

*源码：*
```pascal
program pv05;
var i: integer;
begin
  for i := 1 to 10 do
    i := i + 1
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    ...
    CompoundStmt
      ForStmt
        Identifier (i)
        Literal
        Literal
        AssignStmt
          Identifier (i)
          BinaryExpr
            Identifier (i)
            Literal
```

`ForStmt` 包含循环变量 `Identifier (i)`、起始值 `Literal`、终止值 `Literal` 和循环体。第四个参数为 `false`（递增方向，to 非 downto）。

=== pv06_for_downto

for-downto 递减循环语句。

*源码：*
```pascal
program pv06;
var i: integer;
begin
  for i := 10 downto 1 do
    i := i - 1
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    ...
    CompoundStmt
      ForStmt
        Identifier (i)
        Literal
        Literal
        AssignStmt
          Identifier (i)
          BinaryExpr
            Identifier (i)
            Literal
```

`ForStmt` 的第四个参数为 `true`（递减方向，downto），与 pv05 形成正交对照。

=== pv07_proc_func

过程与函数声明，参数传递和调用。

*源码：*
```pascal
program pv07;

procedure p(x: integer);
begin
  x := x + 1
end;

function f(y: integer): integer;
begin
  f := y
end;

begin
  p(1);
  write(f(2))
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    List
    List
    List
    List
      ProcDecl
        List
          ParamDecl
            List
              Identifier (x)
            Identifier (integer)
        Block
          List
          List
          CompoundStmt
            AssignStmt
              Identifier (x)
              BinaryExpr
                Identifier (x)
                Literal
      FuncDecl
        List
          ParamDecl
            List
              Identifier (y)
            Identifier (integer)
        Block
          List
          List
          CompoundStmt
            AssignStmt
              Identifier (f)
              Identifier (y)
    CompoundStmt
      ProcCall
        Literal
      ProcCall
        ProcCall
          Literal
```

`ProcDecl` 和 `FuncDecl` 各有参数列表、返回类型和体 block。过程调用 `p(1)` 和函数调用 `f(2)` 均表示为 `ProcCall` 节点，嵌套调用 `write(f(2))` 正确构建了 `ProcCall → ProcCall → Literal` 的嵌套 AST。

=== pv08_array

数组声明与下标访问。

*源码：*
```pascal
program pv08;
var a: array[1..10] of integer;
    i: integer;
begin
  a[1] := 2;
  i := a[1]
end.
```

*AST 输出：*
```
Parse succeeded.
Program
  Block
    ...
      VarDecl
        List
          Identifier (a)
        ArrayType
          Literal
          Literal
          Identifier (integer)
      VarDecl
        List
          Identifier (i)
        Identifier (integer)
    ...
    CompoundStmt
      AssignStmt
        ArrayAccess
          Identifier (a)
          Literal
        Literal
      AssignStmt
        Identifier (i)
        ArrayAccess
          Identifier (a)
          Literal
```

`ArrayType` 节点包含下界 `Literal`、上界 `Literal` 和元素类型 `Identifier (integer)`。`ArrayAccess` 节点在赋值左值（写入 `a[1]`）和右值（读取 `a[1]`）位置上均正确构建。

== 非法用例（parser_invalid/，共 8 个）

=== pi01_missing_dot

程序末尾缺少句点。

*源码：*
```pascal
program pi01;
begin
end
```

*错误输出：*
```
Parse error at 3:4: syntax error
Parsing failed.
```

解析器在文件末尾检测到缺少 `.`，报错位置精确指向第 3 行第 4 列（`end` 之后）。

=== pi02_missing_decl_semicolon

变量声明之间缺少分号。

*源码：*
```pascal
program pi02;
var a: integer
    b: integer;
begin
  a := 1
end.
```

*错误输出：*
```
Parse error at 3:5 near 'b': syntax error
Parsing failed.
```

`a: integer` 后缺少分号，解析器在看到下一行开头的 `b` 时报错，行列信息准确。

=== pi03_unmatched_begin_end

`begin`/`end` 不配对。

*源码：*
```pascal
program pi03;
begin
  begin
    write(1)
end.
```

*错误输出：*
```
Parse error at 5:4 near '.': syntax error
Parsing failed.
```

内层 `begin` 缺少对应的 `end`，解析器在遇到程序结尾的 `.` 时检测到配对不匹配。

=== pi04_if_missing_then

`if` 语句缺少 `then` 关键字。

*源码：*
```pascal
program pi04;
var a: integer;
begin
  if a = 1
    a := 2
end.
```

*错误输出：*
```
Parse error at 5:5 near 'a': syntax error
Parsing failed.
```

`if a = 1` 后缺少必需的 `then`，解析器在看到赋值左值 `a` 时报错。

=== pi05_for_missing_do

`for` 循环缺少 `do` 关键字。

*源码：*
```pascal
program pi05;
var i: integer;
begin
  for i := 1 to 10
    i := i + 1
end.
```

*错误输出：*
```
Parse error at 5:5 near 'i': syntax error
Parsing failed.
```

`for i := 1 to 10` 后缺少 `do`，解析器在看到循环体中的 `i` 时报错。

=== pi06_array_missing_range_op

数组声明中范围操作符错误（`.` 替代 `..`）。

*源码：*
```pascal
program pi06;
var a: array[1.10] of integer;
begin
  a[1] := 2
end.
```

*错误输出：*
```
Parse error at 2:18 near ']': syntax error
Parsing failed.
```

`1.10` 被解析为一个实数而非范围 `1..10`，解析器在遇到 `]` 时发现语法不匹配。

=== pi07_proc_param_missing_colon

过程参数声明缺少冒号。

*源码：*
```pascal
program pi07;
procedure p(x integer);
begin
end;
begin
end.
```

*错误输出：*
```
Parse error at 2:15 near 'integer': syntax error
Parsing failed.
```

`x` 和 `integer` 之间缺少 `:`，解析器在看到 `integer` 关键字时报错。

=== pi08_call_missing_rparen

过程调用缺少右括号。

*源码：*
```pascal
program pi08;
begin
  write(1
end.
```

*错误输出：*
```
Parse error at 4:1 near 'end': syntax error
Parsing failed.
```

`write(1` 缺少 `)`，解析器在遇到下一行的 `end` 时报错。

== 测试结果汇总

#figure(
  table(
    columns: (auto, auto, auto),
    [用例], [类型], [结果],
    [pv01_minimal], [正例], [AST 构建正确],
    [pv02_assign], [正例], [AST 构建正确],
    [pv03_if_else], [正例], [AST 构建正确],
    [pv04_while], [正例], [AST 构建正确],
    [pv05_for_to], [正例], [AST 构建正确],
    [pv06_for_downto], [正例], [AST 构建正确],
    [pv07_proc_func], [正例], [AST 构建正确],
    [pv08_array], [正例], [AST 构建正确],
    [pi01_missing_dot], [负例], [正确检测到语法错误],
    [pi02_missing_decl_semicolon], [负例], [正确检测到语法错误],
    [pi03_unmatched_begin_end], [负例], [正确检测到语法错误],
    [pi04_if_missing_then], [负例], [正确检测到语法错误],
    [pi05_for_missing_do], [负例], [正确检测到语法错误],
    [pi06_array_missing_range_op], [负例], [正确检测到语法错误],
    [pi07_proc_param_missing_colon], [负例], [正确检测到语法错误],
    [pi08_call_missing_rparen], [负例], [正确检测到语法错误],
  ),
  caption: [语法分析测试结果汇总（16 个用例全部通过）]
)
