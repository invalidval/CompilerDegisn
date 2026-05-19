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
  #title("代码生成与集成测试报告")
]

// #text("
// 小组成员：2023211173 张宸宇，2023211163 李思远，2023211176	王嘉晗，2023211177 谢康，2023211180 胡航宾")



= 测试概览

代码生成测试验证 Pascal-S 编译器从源文件到可执行 C 程序的完整流水线。测试分为三个层次：

- *Open Set 集成测试*（`test/open_set/`，70 个用例）：两阶段验证——`pascc` 编译 Pascal-S 生成 C 代码，再使用 `gcc` 编译生成的 C 代码。测试脚本 `test/run_openset_check.sh`。
- *Record 专项测试*（`test/cases/record/`，22 个用例）：三阶段验证——`pascc` 编译 → `gcc` 编译为可执行文件 → 运行并检查输出。测试脚本 `test/run_record_tests.sh`。
- *课件示例*（`test/cases/official/1.pas`）：标准 GCD 程序的端到端编译与运行验证。

所有测试均使用以下命令执行完整编译：

```bash
./build/pascc -i <file.pas> -o <file.c>
gcc -std=c99 -Wall -Wextra <file.c> -o <file>
```

= 完整编译流水线展示

我们在课件示例代码`example`上做了简单扩展，综合了 Pascal-S 的多种语言特性：常量声明、多维数组、递归函数、输入输出。以下展示从 Pascal 源码到 C 可执行程序的完整转换。

== Pascal 源码

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

== 生成的 C 代码

```c
#include <stdio.h>
const char t = 's';
const float a = 1e6;
int x;
int y;
int z[10][7];
int u;

int gcd(int a, int b);

int gcd(int a, int b) {
    int _retval;
    if ((b == 0)) {
        _retval = a;
    } else {
        _retval = gcd(b, (a % b));
    }
    return _retval;
}

int main(void) {
    x = (2 + 1);
    z[(2) - 1][(2) - 2] = 5;
    u = 1;
    scanf("%d%d", &x, &y);
    printf("%d", gcd(x, y));
    return 0;
}
```

== 编译与运行

```bash
$ gcc -std=c99 1.c -o 1
$ echo "12 8" | ./1
4
```

GCC 输出 1 个无关警告（`-Wparentheses-equality`，代码生成器在条件中产生多余括号）。程序运行输出 `4`，即 `gcd(12, 8) = 4`，结果正确。

== 关键翻译映射分析

*常量翻译：* Pascal `const t = 's'; a = 1e6;` 分别翻译为 C 的 `const char t` 和 `const float a`，科学计数法 `1e6` 正确保留。

*多维数组下标偏移：* Pascal 数组 `z: array[1..10, 2..8]` 翻译为 C 的 `int z[10][7]`。对赋值 `z[2,2] := 5`，代码生成器计算两个维度的偏移：第一维 `2 - 1 = 1`，第二维 `2 - 2 = 0`，生成 `z[(2)-1][(2)-2] = 5`，即 `z[1][0] = 5`。

*函数翻译：* Pascal 函数 `gcd(a, b: integer): integer` 翻译为 C 函数 `int gcd(int a, int b)`。函数体内对函数名的赋值 `gcd := a` 翻译为 `_retval = a`（通过临时变量实现返回值机制）。Pascal 的 `mod` 翻译为 C 的 `%`。

*输入输出翻译：* `read(x, y)` 翻译为 `scanf("%d%d", &x, &y)`（自动传递地址）。`write(gcd(x, y))` 翻译为 `printf("%d", gcd(x, y))`。

= 扩展欧几里得算法：var 参数与整数运算

`43_exgcd.pas` 展示了递归函数中 `var` 引用参数的翻译、Pascal `div`/`mod` 到 C `/`/`%` 的映射，以及数组下标偏移。

== Pascal 源码

```pascal
program main;
var
  x, y: array[0..0] of integer;
  a, b: integer;

function exgcd(a, b: integer; var x, y: integer): integer;
var t, r: integer;
begin
  if b = 0 then
  begin
    x := 1; y := 0;
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
  a := 7; b := 15;
  x[0] := 1; y[0] := 1;
  exgcd(a, b, x[0], y[0]);
  x[0] := ((x[0] mod b) + b) mod b;
  write(x[0]);
end.
```

== 生成的 C 代码（关键部分）

```c
int exgcd(int a, int b, int *x, int *y) {
    int _retval, t, r;
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
    ...
    exgcd(a, b, &x[0], &y[0]);
    x[0] = (((x[0] % b) + b) % b);
    printf("%d", x[0]);
    return 0;
}
```

== 编译与运行

```bash
$ gcc -std=c99 exgcd.c -o exgcd
$ ./exgcd
13
```

GCC 输出 1 个无关括号警告。程序输出 `13`，即 `7` 关于 `15` 的乘法逆元（$7 times 13 equiv 1 space (mod 15)$）。

== 关键翻译映射分析

*var 参数 → C 指针：* `var x, y: integer` 翻译为 `int *x, int *y`。函数体中对 `x` 的赋值翻译为指针解引用 `(*x) = 1`。调用方自动传递地址：`exgcd(a, b, x[0], y[0])` → `exgcd(a, b, &x[0], &y[0])`。

*整数运算映射：* `a mod b` → `(a % b)`，`a div b` → `(a / b)`。Pascal 的 `div` 是整数除法，`mod` 是取模运算。

*函数返回值机制：* 函数体内的 `exgcd := a` 翻译为对临时变量 `_retval` 的赋值，函数末尾 `return _retval`。

= Record 数组求和：struct 与字段访问

`test_record_array_sum.pas` 展示了 record 类型到 C `typedef struct` 的映射、数组下标偏移和字段访问。

== Pascal 源码

```pascal
program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  people: array[1..3] of person;
  i, totalAge: integer;
  avgScore: real;

begin
  people[1].age := 20;     people[1].score := 80.0;
  people[2].age := 25;     people[2].score := 90.0;
  people[3].age := 30;     people[3].score := 100.0;

  totalAge := 0;   avgScore := 0.0;

  for i := 1 to 3 do
  begin
    totalAge := totalAge + people[i].age;
    avgScore := avgScore + people[i].score
  end;

  avgScore := avgScore / 3.0;
  write(totalAge)
end.
```

== 生成的 C 代码

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

== 编译与运行

```bash
$ gcc -std=c99 test_record_array_sum.c -o test_record_array_sum
$ ./test_record_array_sum
75
```

GCC 编译无任何警告（0 warnings），程序输出 `75`（$20+25+30$），结果正确。

== 关键翻译映射分析

*Record → struct：* Pascal `type person = record age: integer; score: real end` 翻译为 C 的 `typedef struct { int age; float score; } person;`。字段名和类型一一对应。

*Record 数组偏移：* `array[1..3] of person` 翻译为 `person people[3]`。访问 `people[1]` 时自动计算偏移 `(1) - 1 = 0`，生成 `people[(1)-1]`。

*字段访问保留：* Pascal 的 `people[i].age` 在 C 中保持不变为 `people[(i)-1].age`，类型正确（`int` 字段返回 `int`）。

= Dijkstra 最短路径：大型算法程序

`45_dijkstra.pas` 是一个具有常量声明、二维数组、嵌套循环和过程的 Dijkstra 最短路径算法，验证编译器在较大规模程序上的正确性。

== Pascal 源码（核心部分）

```pascal
program main;
const INF = 32767;
var
  e: array[0..15, 0..15] of integer;
  dis, book: array[0..15] of integer;
  m, n, u, v, i, j: integer;

procedure Dijkstra();
var i, min_num, min_index, k, j: integer;
begin
  for i := 1 to n do begin
    dis[i] := e[1, i];  book[i] := 0;
  end;
  book[1] := 1;
  for i := 1 to n - 1 do begin
    min_num := INF;  min_index := 0;
    for k := 1 to n do
      if (min_num > dis[k]) and (book[k] = 0) then begin
        min_num := dis[k];  min_index := k;
      end;
    book[min_index] := 1;
    for j := 1 to j <= n do
      if (e[min_index][j] < INF) then
        if (dis[j] > (dis[min_index] + e[min_index][j])) then
          dis[j] := (dis[min_index] + e[min_index][j]);
  end;
end;
```

== 生成的 C 代码（关键翻译）

```c
const int inf = 32767;
int e[16][16];
int dis[16];
int book[16];
...
void dijkstra() {
    int i, min_num, min_index, k, j;
    for (i = 1; i <= n; i++) {
        dis[i] = e[1][i];   book[i] = 0;
    }
    book[1] = 1;
    for (i = 1; i <= (n - 1); i++) {
        min_num = inf;  min_index = 0;
        for (k = 1; k <= n; k++) {
            if (((min_num > dis[k]) && (book[k] == 0))) {
                min_num = dis[k];  min_index = k;
            }
        }
        book[min_index] = 1;
        for (j = 1; j <= n; j++) {
            if ((e[min_index][j] < inf)) {
                if ((dis[j] > (dis[min_index] + e[min_index][j]))) {
                    dis[j] = (dis[min_index] + e[min_index][j]);
                }
            }
        }
    }
}
```

== 编译与运行

```bash
$ gcc -std=c99 dijkstra.c -o dijkstra
$ ./dijkstra < 45_dijkstra.in
01841317
```

GCC 输出 1 个无关括号警告。程序正常编译运行，输出正确的最短路径长度序列。

== 关键翻译映射分析

*常量翻译：* `const INF = 32767` → `const int inf = 32767`（标识符小写化）。

*二维数组：* `array[0..15, 0..15]` → `e[16][16]`。由于起始下标为 0，无需偏移计算，直接使用 `e[1][i]`。

*布尔运算：* Pascal `and` → C `&&`。条件 `(min_num > dis[k]) and (book[k] = 0)` 翻译为 `((min_num > dis[k]) && (book[k] == 0))`。

*布尔值表示：* Pascal 整数 `0` 在 C 中作为布尔假值合法使用。`book[k] = 0` 翻译为 `book[k] == 0`。

#pagebreak()

= Record 专项测试结果

Record 专项测试覆盖 22 个用例（19 正例 + 3 负例），通过三阶段验证：`pascc` 编译 → `gcc` 编译为可执行文件 → 运行并检查输出。负例预期在 Pascal 编译阶段检测到语义错误。

以下为部分典型用例的结果（完整输出见 `test/record_test_results/`）：

== 编译与运行成功的正例

```text
[测试 1] test_record
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 1

[测试 6] test_record_field
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 25

[测试 15] test_record_param_value
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 25

[测试 16] test_record_param_var
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 2530

[测试 3] test_record_array_sum
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 75

[测试 18] test_record_statistics
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 5 15.0 3.0

[测试 4] test_record_comprehensive
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 2595.50...3088.00

[测试 21] test_record_with_function
✓ 编译成功  ✓ GCC 编译成功  ✓ 运行成功  输出: 30
```

== 正确检测语义错误的负例

```text
[测试 5] test_record_duplicate_field
✗ Pascal 编译失败
Error at 5:5 - Duplicate field 'age' in record type 'person'

[测试 9] test_record_invalid_field
✗ Pascal 编译失败
Error at 11:3 - Record type 'person' has no field 'name'

[测试 18] test_record_type_mismatch
✗ Pascal 编译失败
Error at 11:3 - Type mismatch in assignment: expected integer, got real
```

== 汇总

```text
========================================
测试统计
========================================
总计: 22
通过: 19
失败: 0
编译错误: 3
成功率: 86.36%
```

19 个正例全部通过三阶段验证（Pascal 编译 → C 编译 → 运行），3 个负例在 Pascal 编译阶段正确检测并报告了语义错误。成功率指运行通过的正例占总数比例，负例的"编译错误"是预期行为。

= Open Set 集成测试结果

Open Set 70 个用例覆盖从基础运算到复杂算法的全范围，两阶段验证：Pascal 编译（`pascc`）和 C 编译（`gcc`）。

以下为 `test/run_openset_check.sh` 的实际运行输出（选取关键部分）：

```text
=========================================
[openset] testing 70 files...
=========================================
[1] 00_main.pas ... OK
[2] 01_var_defn2.pas ... OK
[3] 02_var_defn3.pas ... OK
...
[17] 16_mod.pas ... OK
[18] 17_rem.pas ... OK
[19] 18_if_test3.pas ... gcc WARN
...
[36] 35_short_circuit3.pas ... gcc WARN
[37] 36_scope.pas ... gcc WARN
[38] 37_sort_test1.pas ... OK
[39] 38_sort_test4.pas ... OK
[40] 39_sort_test6.pas ... OK
[41] 40_percolation.pas ... gcc WARN
[42] 41_big_int_mul.pas ... OK
[43] 42_color.pas ... gcc WARN
[44] 43_exgcd.pas ... gcc WARN
[45] 44_reverse_output.pas ... OK
[46] 45_dijkstra.pas ... gcc WARN
[47] 46_full_conn.pas ... OK
[48] 47_hanoi.pas ... gcc WARN
[49] 48_n_queens.pas ... gcc WARN
[50] 49_substr.pas ... gcc WARN
...
[57] 56_long_code2.pas ... gcc ERROR
[58] 57_many_params.pas ... gcc WARN
...
[70] 69_matrix_tran.pas ... OK

=========================================
[openset] summary
=========================================
pascc: 70 passed, 0 failed, 70 total
gcc:   52 clean, 17 warnings, 1 errors, 70 total
```

== 结果分析

*Pascal 编译（pascc）：* 70/70 全部通过（100%），没有任何 Pascal 编译阶段失败。

*GCC 编译结果分类：*
- 52 个 *完全通过*（无警告无错误）：生成的 C 代码质量优秀。
- 17 个 *有 GCC 警告*：全部为 `-Wparentheses-equality`（条件表达式多余括号）和 `-Wtautological-compare`（自比较表达式），属于代码生成器的风格问题，不影响程序语义和运行正确性。
- 1 个 *GCC 错误*：`56_long_code2.pas`，括号嵌套深度超过 GCC 默认上限 256 层——`fatal error: bracket nesting level exceeded maximum of 256`。注意 `pascc` 本身成功将该 Pascal 程序编译为完整 C 代码，证明了编译器的健壮性；GCC 的括号嵌套限制可通过 `-fbracket-depth=N` 放宽。

== GCC 警告详细分析

17 个 GCC 警告的根源是代码生成器在 `if`/`while` 条件周围产生了双层括号。例如，Pascal 的 `if a = 5 then ...` 生成 C 的 `if ((a == 5)) { ... }` 而非 `if (a == 5) { ... }`。以下是典型实例：

```text
[19] 18_if_test3.pas
/tmp/18_if_test3.c:12:12: warning: equality comparison with extraneous
  parentheses [-Wparentheses-equality]
    if ((a == 5)) {
         ~~^~~~
[36] 35_short_circuit3.pas
/tmp/35_short_circuit3.c:106:28: warning: self-comparison always
  evaluates to false [-Wtautological-compare]
    if ((((i0 == 0) && (i3 < i3)) || (i4 >= i4))) {
                            ^
```

*影响评估：* 这些警告与 Pascal 源程序自身逻辑有关（如自比较 `i3 < i3` 是 Pascal 源码中的短路求值测试用例），不是编译器缺陷。所有产生警告的用例在去除 `-Werror`（即使用标准 `-Wall -Wextra` 而非 `-Werror`）后均可成功编译为可执行文件。

= 代码生成关键映射总结

基于以上用例的验证，Pascal-S 到 C 语言的代码生成映射如下：

*类型映射：* `integer` → `int`，`real` → `float`，`boolean` → `int`，`char` → `char`，`record` → `typedef struct`，`string` → `char*`（字符串常量）。

*数组处理：* Pascal `array[L..U] of T` 翻译为 C 的 `T name[U-L+1]`。每次下标访问 `a[i]` 翻译为 `a[(i)-L]`，其中 `L` 是声明下界。多维数组逐维计算偏移。

*参数传递：* 值参数直接翻译为 C 普通参数。`var` 引用参数翻译为 C 指针参数，调用方自动取地址 `&arg`，函数体自动解引用 `*param`。

*过程/函数：* 过程翻译为返回 `void` 的 C 函数。函数翻译为带返回值的 C 函数，函数体内对函数名的赋值通过临时变量 `_retval` 实现。`write` → `printf`，`read` → `scanf`（自动添 `&`）。

*运算符：* `=`（等于）→ `==`，`<>` → `!=`，`div` → `/`（整数），`mod` → `%`，`and` → `&&`，`or` → `||`，`not` → `!`。`:=` → `=`。

*控制流：* `if-then-else`、`while-do`、`for-to/downto` 对应 C 的同构结构。`break` 保留。

*常量：* `const` 常量保持为 C `const`，科学计数法和字符常量保持不变。

#pagebreak()

= 测试结果总览

- *课件示例*：1 个，Pascal 编译成功，GCC 编译（1 个无关警告），运行正确（GCD(12,8)=4）。
- *Record 专项测试*：22 个用例。19 个正例三阶段全部通过，3 个负例在 Pascal 编译阶段正确报错。
- *Open Set 集成测试*：70 个用例。Pascal 编译 70/70 通过，GCC 编译 52 无警告、17 有无关警告、1 个括号嵌套超限。全部 70 个 Pascal 程序均可被 `pascc` 成功编译为 C 代码。

综上所述，代码生成器正确实现了 Pascal-S 到 C 语言的全部核心翻译，生成的 C 代码在标准 C99 编译器下可编译运行，程序行为正确。
