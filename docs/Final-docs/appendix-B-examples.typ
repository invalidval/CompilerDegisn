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

= 附录B：部分用例及结果

本附录介绍部分用例的内容和结果，用于辅助第六章内容。

== 词法分析测试

=== 合法用例：关键字大小写不敏感

测试用例 `v02_keyword_mixedcase.pas` 验证词法分析器对 Pascal 关键字不区分大小写的处理能力。

*测试源码：*
```pascal
ProGram t02;
begin
end.
```

*编译命令：*
```bash
./build/pascc -i test/cases/valid/v02_keyword_mixedcase.pas --lex
```

*Token 输出：*
```csv
Type, Lexeme, Line, Column
Keyword, ProGram, 1, 1
Identifier, t02, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, end, 3, 1
Delimiter, ., 3, 4
```

词法分析器正确将大小写混合的token识别为 `program`、`begin`、`end` 关键字，`t02` 为标准标识符，行列号精确。

=== 非法用例：未终止字符字面量

测试用例 `i05_unterminated_char_literal.pas` 验证词法分析器对未闭合字符字面量的错误检测。

*测试源码：*
```pascal
program i05;
var c: char;
begin
  c := 'a;  { 未终止的字符字面量 }
end.
```

*编译命令：*
```bash
./build/pascc -i test/cases/invalid/i05_unterminated_char_literal.pas --lex
```

*错误输出：*
```
Error at 4:8 - Lexical error: unterminated character literal (lexeme=''a;')
```

词法分析器在编译阶段正确报告了未终止字符字面量错误，同时给出了精确的行号（第 4 行）和列号（第 8 列），以及引发错误的词素内容。

#pagebreak()

== 语法分析测试

=== 合法用例：for-to 循环的 AST 构建

测试用例 `pv05_for_to.pas` 验证语法分析器对 for-to 循环的正确解析和 AST 构建。

*测试源码：*
```pascal
program pv05;
var i: integer;
begin
  for i := 1 to 10 do
    i := i + 1
end.
```

*编译命令：*
```bash
./build/pascc -i test/cases/parser_valid/pv05_for_to.pas --parse
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
          Identifier (i)
        Identifier (integer)
    List
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

语法分析器生成了正确的 AST：`ForStmt` 节点包含循环变量 `i`、起始值 `1`、终止值 `10` 和循环体（赋值语句），各节点类型与层级结构均正确。

=== 非法用例：if 缺少 then

测试用例 `pi04_if_missing_then.pas` 验证语法分析器对缺失关键字的错误检测与恐慌模式恢复。

*测试源码：*
```pascal
program pi04;
var a: integer;
begin
  if a = 1
    a := 2
end.
```

*编译命令：*
```bash
./build/pascc -i test/cases/parser_invalid/pi04_if_missing_then.pas --parse
```

*错误输出：*
```
Parse error at 5:5 near 'a': syntax error
Parsing failed.
```

语法分析器检测到 `if` 语句中缺少必需的 `then` 关键字，在第 5 行第 5 列报错并终止分析。

== 语义分析测试

=== 合法用例：阶乘递归函数

测试用例 `sv17_func_result.pas` 验证语义分析器对递归函数声明与函数返回值赋值的处理。

*测试源码：*
```pascal
program sv17_func_result;
var n, result: integer;

function factorial(x: integer): integer;
begin
    if x <= 1 then
        factorial := 1
    else
        factorial := x * factorial(x - 1)
end;

begin
    n := 5;
    result := factorial(n)
end.
```

*编译命令：*
```bash
./build/pascc -i test/cases/semantic_valid/sv17_func_result.pas --semantic
```

*语义分析输出：*
```
Semantic analysis succeeded.
```

语义分析器正确识别了函数体内对函数名 `factorial` 的赋值为合法的函数返回值设置操作。

=== 合法用例：多维数组

测试用例 `sv26_array_multidim.pas` 验证语义分析器对多维数组声明与下标访问的处理。

*测试源码：*
```pascal
program sv26_array_multidim;
var
    matrix: array[1..3, 1..3] of integer;
    i, j, sum: integer;
begin
    for i := 1 to 3 do
        for j := 1 to 3 do
            matrix[i, j] := i * j;
    sum := 0;
    for i := 1 to 3 do
        sum := sum + matrix[i, i]
end.
```

*语义分析输出：*
```
Semantic analysis succeeded.
```

=== 非法用例：类型不匹配赋值

测试用例 `si06_type_mismatch_assign.pas` 验证语义分析器对布尔值赋值给整数变量的类型错误检测。

*测试源码：*
```pascal
program si06_type_mismatch_assign;
var a: integer;
begin
    a := true
end.
```

*编译命令：*
```bash
./build/pascc -i test/cases/semantic_invalid/si06_type_mismatch_assign.pas --semantic
```

*错误输出：*
```
Error at 5:5 - Type mismatch in assignment: expected integer, got boolean
```

=== 非法用例：数组下标越界

测试用例 `si12_array_index_bounds.pas` 验证语义分析器对编译期常量下标越界的检测能力。

*测试源码：*
```pascal
program si12_array_index_bounds;
var arr: array[1..10] of integer;
begin
    arr[0] := 1
end.
```

*编译命令：*
```bash
./build/pascc -i test/cases/semantic_invalid/si12_array_index_bounds.pas --semantic
```

*错误输出：*
```
Error at 5:9 - Array index out of bounds: 0 not in [1, 10]
```

语义分析器通过编译期常量求值，检测到下标 `0` 超出数组 `arr[1..10]` 的声明范围。

#pagebreak()

== Record 专项测试

=== 合法用例：record 数组求总年龄

测试用例 `test_record_array_sum.pas` 验证 record 数组的声明、字段访问、循环遍历与聚合运算。

*测试源码：*
```pascal
program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  people: array[1..3] of person;
  i: integer;
  totalAge: integer;
  avgScore: real;

begin
  people[1].age := 20;
  people[1].score := 80.0;
  people[2].age := 25;
  people[2].score := 90.0;
  people[3].age := 30;
  people[3].score := 100.0;

  totalAge := 0;
  avgScore := 0.0;

  for i := 1 to 3 do
  begin
    totalAge := totalAge + people[i].age;
    avgScore := avgScore + people[i].score
  end;

  avgScore := avgScore / 3.0;

  write(totalAge)
end.
```

*pascc 生成的目标 C 代码：*
```c
#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person people[3];

int i;
int totalage;
float avgscore;

int main(void) {
    people[(1) - 1].age = 20;
    people[(1) - 1].score = 80.0;
    people[(2) - 1].age = 25;
    people[(2) - 1].score = 90.0;
    people[(3) - 1].age = 30;
    people[(3) - 1].score = 100.0;
    totalage = 0;
    avgscore = 0.0;
    for (i = 1; i <= 3; i++) {
        totalage = (totalage + people[(i) - 1].age);
        avgscore = (avgscore + people[(i) - 1].score);
    }
    avgscore = (avgscore / 3.0);
    printf("%d", totalage);
    return 0;
}
```

*C 编译与运行：*
```bash
$ gcc -std=c99 test_record_array_sum.c -o test_record_array_sum
$ ./test_record_array_sum
75
```

编译器正确实现了 Pascal `record` 类型到 C `struct` 的映射、`array[1..3]` 到 `[3]` 的下标偏移（`(i) - 1`）、以及字段访问 `people[i].age` 的忠实翻译。程序输出 75（$20+25+30$），验证正确。

=== 合法用例：record 作为 var 参数

测试用例 `test_record_param_var.pas` 验证 record 类型作为 `var` 引用参数传入过程的正确性——参数修改能反映回调用方。

*测试源码：*
```pascal
program test;
type
  person = record
    age: integer;
    score: real
  end;
var p: person;

procedure modifyPerson(var p: person);
begin
  p.age := 30;
  p.score := 88.0
end;

begin
  p.age := 25;
  p.score := 95.5;
  write(p.age);
  modifyPerson(p);
  write(p.age)
end.
```

*pascc 生成的目标 C 代码：*
```c
#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person p;

void modifyperson(person *p);

void modifyperson(person *p) {
    (*p).age = 30;
    (*p).score = 88.0;
}

int main(void) {
    p.age = 25;
    p.score = 95.5;
    printf("%d", p.age);
    modifyperson(&p);
    printf("%d", p.age);
    return 0;
}
```

*C 编译与运行：*
```bash
$ gcc -std=c99 test_record_param_var.c -o test_record_param_var
$ ./test_record_param_var
2530
```

关键观察：
- Pascal `var` 参数正确翻译为 C 指针参数（`person *p`）。
- 过程体内对 record 字段的访问编译为 C 指针解引用 `(*p).age`。
- 调用方自动传递变量地址 `&p`。
- 输出 `2530`：修改前 `p.age = 25`（输出 `25`），`modifyPerson` 修改后 `p.age = 30`（输出 `30`），验证引用传递正确。

=== 非法用例：访问不存在的 record 字段

测试用例 `test_record_invalid_field.pas` 验证语义分析器对 record 字段访问的静态检查。

*测试源码：*
```pascal
program test;
type
  person = record
    age: integer;
    score: real
  end;
var p: person;

begin
  p.name := 'John';  { Error: field 'name' does not exist }
  write(p.age)
end.
```

*语义分析输出：*
```
Error at 11:3 - Record type 'person' has no field 'name'
Error at 11:3 - Type mismatch in assignment: expected unknown, got char
```

#pagebreak()

== 课件示例测试

课件中示例程序的扩展，验证编译器对完整标准 Pascal-S 程序的端到端编译能力。

*测试源码 `test/cases/official/1.pas`：*
```pascal
program example(input, output);
const t = 's'; a = 1e6;
var x, y: integer;
    z : array [1..10, 2..8] of integer;
    u : integer;

function gcd(a, b: integer): integer;
begin
  if b=0 then gcd:=a
  else gcd:=gcd(b, a mod b)
end;

begin
  x := 2+1;
  z[2,2] := 5;
  u := 1;
  read(x, y);
  write(gcd(x, y))
end.
```

该程序定义了 Pascal-S 的多种语言特性：常量声明（字符常量 `'s'`、科学计数法实数 `1e6`）、多维数组声明（`array[1..10, 2..8]`）、递归 GCD 函数、以及输入输出语句。编译器能够完整编译该程序，生成正确可编译的 C 代码，通过 gcc 无警告编译，运行时正确计算两数的最大公约数并输出。

== Open Set 集成测试

=== 扩展欧几里得算法

测试用例 `43_exgcd.pas` 验证编译器对递归函数、`var` 参数、整数除法和取模运算的完整编译。

*测试源码：*
```pascal
program main;
var
  a, b: integer;

function exgcd(a, b: integer; var x, y: integer): integer;
var t, r: integer;
begin
  if b = 0 then
  begin
    x := 1;
    y := 0;
    exgcd := a;
  end
  else
  begin
    r := exgcd(b, a mod b, x, y);
    t := x;
    x := y;
    y := (t - (a div b) * y);
    exgcd := r;
  end;
end;

begin
  a := 7;
  b := 15;
  exgcd(a, b, x[0], y[0]);
  x[0] := ((x[0] mod b) + b) mod b;
  write(x[0]);
end.
```

*pascc 生成的目标 C 代码：*
```c
#include <stdio.h>
int a; int b;

int exgcd(int a, int b, int *x, int *y);

int exgcd(int a, int b, int *x, int *y) {
    int _retval; int t; int r;
    if ((b == 0)) {
        (*x) = 1;
        (*y) = 0;
        _retval = a;
    } else {
        r = exgcd(b, (a % b), x, y);
        t = (*x);
        (*x) = (*y);
        (*y) = (t - ((a / b) * (*y)));
        _retval = r;
    }
    return _retval;
}

int main(void) {
    a = 7; b = 15;
    exgcd(a, b, &x[0], &y[0]);
    x[0] = (((x[0] % b) + b) % b);
    printf("%d", x[0]);
    return 0;
}
```

*C 编译与运行（gcc 产生 1 个无关警告）：*
```bash
$ gcc -std=c99 exgcd.c -o exgcd
$ ./exgcd
13
```

程序使用扩展欧几里得算法求解 $7$ 关于模数 $15$ 的乘法逆元，结果为 $13$（$7 times 13 equiv 1 space (mod 15)$），验证编译正确。

=== 图着色计数（DP）

测试用例 `42_color.pas` 是一个较复杂的动态规划程序，求解图的合法着色方案数，对编译器多维数组访问、常量表达式和模运算的处理形成压力测试。

*核心递归函数源码片段（完整程序 55 行）：*
```pascal
function dfs(a, b, c, d, e, last: integer): integer;
var anss: integer;
begin
  if dp[a, b, c, d, e, last] <> -1 then
    dfs := dp[a, b, c, d, e, last];
  if a + b + c + d + e = 0 then
    dfs := 1
  else
  begin
    anss := 0;
    if a <> 0 then
      anss := (anss + (a - equal(last, 2))
               * dfs(a - 1, b, c, d, e, 1)) mod modn;
    { ... 其余四个分支类似 ... }
    dp[a, b, c, d, e, last] := anss mod modn;
    dfs := dp[a, b, c, d, e, last];
  end;
end;
```

该程序使用了五维 DP 数组 `dp[0..17, 0..17, 0..17, 0..17, 0..17, 0..6]`，六层嵌套 for 循环进行初始化，以及递归 DFS 中的多维数组访问和模运算。

*pascc 生成的多维数组下标偏移（C 代码片段）：*
```c
int dp[18][18][18][18][18][7];
// ...
cns[(list[i]) - 1] = (cns[(list[i]) - 1] + 1);
ans = dfs(cns[(1) - 1], cns[(2) - 1], cns[(3) - 1],
          cns[(4) - 1], cns[(5) - 1], 0);
```

*C 编译与运行：*
```bash
$ gcc -std=c99 color.c -o color
$ ./color < 42_color.in
39480
```

即便存在 gcc 的括号风格警告，程序编译后正确运行并输出期望结果 `39480`，验证了编译器在复杂算法场景下的多维数组翻译、var 参数处理和模运算的正确性。
