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
  #title("词法分析单元测试报告")
]

// #text("
// 小组成员：2023211173 张宸宇，2023211163 李思远，2023211176	王嘉晗，2023211177 谢康，2023211180 胡航宾")


= 词法分析测试

所有词法测试均使用以下命令执行（`--lex` 模式仅输出 Token 流，不做语法/语义分析）：

```bash
./build/pascc -i <file.pas> --lex
```

== 合法用例（valid/，共 30 个）

=== gcd
*源码：*
```pascal
program gcd_demo;
var a, b: integer;

function gcd(x, y: integer): integer;
begin
  if y = 0 then
    gcd := x
  else
    gcd := gcd(y, x mod y)
end;

begin
  a := 12;
  b := 18;
  write(gcd(a, b));
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, gcd_demo, 1, 9
Delimiter, ;, 1, 17
Keyword, var, 2, 1
Identifier, a, 2, 5
Delimiter, ,, 2, 6
Identifier, b, 2, 8
Delimiter, :, 2, 9
Keyword, integer, 2, 11
Delimiter, ;, 2, 18
Keyword, function, 4, 1
Identifier, gcd, 4, 10
Delimiter, (, 4, 13
Identifier, x, 4, 14
Delimiter, ,, 4, 15
Identifier, y, 4, 17
Delimiter, :, 4, 18
Keyword, integer, 4, 20
Delimiter, ), 4, 27
Delimiter, :, 4, 28
Keyword, integer, 4, 30
Delimiter, ;, 4, 37
Keyword, begin, 5, 1
Keyword, if, 6, 3
Identifier, y, 6, 6
Operator, =, 6, 8
Number, 0, 6, 10
Keyword, then, 6, 12
Identifier, gcd, 7, 5
Operator, :=, 7, 9
Identifier, x, 7, 12
Keyword, else, 8, 3
Identifier, gcd, 9, 5
Operator, :=, 9, 9
Identifier, gcd, 9, 12
Delimiter, (, 9, 15
Identifier, y, 9, 16
Delimiter, ,, 9, 17
Identifier, x, 9, 19
Keyword, mod, 9, 21
Identifier, y, 9, 25
Delimiter, ), 9, 26
Keyword, end, 10, 1
Delimiter, ;, 10, 4
Keyword, begin, 12, 1
Identifier, a, 13, 3
Operator, :=, 13, 5
Number, 12, 13, 8
Delimiter, ;, 13, 10
Identifier, b, 14, 3
Operator, :=, 14, 5
Number, 18, 14, 8
Delimiter, ;, 14, 10
Keyword, write, 15, 3
Delimiter, (, 15, 8
Identifier, gcd, 15, 9
Delimiter, (, 15, 12
Identifier, a, 15, 13
Delimiter, ,, 15, 14
Identifier, b, 15, 16
Delimiter, ), 15, 17
Delimiter, ), 15, 18
Delimiter, ;, 15, 19
Keyword, end, 16, 1
Delimiter, ., 16, 4
```

=== minimal
*源码：*
```pascal
program minimal;
var a: integer;
begin
  a := 1;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, minimal, 1, 9
Delimiter, ;, 1, 16
Keyword, var, 2, 1
Identifier, a, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, a, 4, 3
Operator, :=, 4, 5
Number, 1, 4, 8
Delimiter, ;, 4, 9
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v01_keyword_lowercase
*源码：*
```pascal
program t01;
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t01, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, end, 3, 1
Delimiter, ., 3, 4
```

=== v02_keyword_mixedcase
*源码：*
```pascal
ProGram t02;
begin
end.
```
*Token输出：*
```csv
Keyword, ProGram, 1, 1
Identifier, t02, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, end, 3, 1
Delimiter, ., 3, 4
```

=== v03_keyword_uppercase
*源码：*
```pascal
PROGRAM t03;
begin
end.
```
*Token输出：*
```csv
Keyword, PROGRAM, 1, 1
Identifier, t03, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, end, 3, 1
Delimiter, ., 3, 4
```

=== v04_identifier_underscore
*源码：*
```pascal
program t04;
var alpha_beta: integer;
begin
  alpha_beta := 1;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t04, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, alpha_beta, 2, 5
Delimiter, :, 2, 15
Keyword, integer, 2, 17
Delimiter, ;, 2, 24
Keyword, begin, 3, 1
Identifier, alpha_beta, 4, 3
Operator, :=, 4, 14
Number, 1, 4, 17
Delimiter, ;, 4, 18
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v05_identifier_with_digits
*源码：*
```pascal
program t05;
var value123: integer;
begin
  value123 := 2;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t05, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, value123, 2, 5
Delimiter, :, 2, 13
Keyword, integer, 2, 15
Delimiter, ;, 2, 22
Keyword, begin, 3, 1
Identifier, value123, 4, 3
Operator, :=, 4, 12
Number, 2, 4, 15
Delimiter, ;, 4, 16
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v06_identifier_leading_underscore
*源码：*
```pascal
program t06;
var _tmp: integer;
begin
  _tmp := 3;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t06, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, _tmp, 2, 5
Delimiter, :, 2, 9
Keyword, integer, 2, 11
Delimiter, ;, 2, 18
Keyword, begin, 3, 1
Identifier, _tmp, 4, 3
Operator, :=, 4, 8
Number, 3, 4, 11
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v07_integer_zero
*源码：*
```pascal
program t07;
var n: integer;
begin
  n := 0;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t07, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, n, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, n, 4, 3
Operator, :=, 4, 5
Number, 0, 4, 8
Delimiter, ;, 4, 9
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v08_integer_max32
*源码：*
```pascal
program t08;
var n: integer;
begin
  n := 2147483647;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t08, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, n, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, n, 4, 3
Operator, :=, 4, 5
Number, 2147483647, 4, 8
Delimiter, ;, 4, 18
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v09_real_basic
*源码：*
```pascal
program t09;
var x: real;
begin
  x := 3.1415;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t09, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, 3.1415, 4, 8
Delimiter, ;, 4, 14
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v10_real_leading_zero
*源码：*
```pascal
program t10;
var x: real;
begin
  x := 0.25;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t10, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, 0.25, 4, 8
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v11_char_letter
*源码：*
```pascal
program t11;
var c: char;
begin
  c := 'a';
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t11, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, c, 2, 5
Delimiter, :, 2, 6
Keyword, char, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, c, 4, 3
Operator, :=, 4, 5
Character, 'a', 4, 8
Delimiter, ;, 4, 11
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v12_char_space
*源码：*
```pascal
program t12;
var c: char;
begin
  c := ' ';
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t12, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, c, 2, 5
Delimiter, :, 2, 6
Keyword, char, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, c, 4, 3
Operator, :=, 4, 5
Character, ' ', 4, 8
Delimiter, ;, 4, 11
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v13_comment_brace_inline
*源码：*
```pascal
program t13; { inline comment }
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t13, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, end, 3, 1
Delimiter, ., 3, 4
```

=== v14_comment_brace_multiline
*源码：*
```pascal
program t14;
{ multi-line
  comment for line/column advancing }
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t14, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 4, 1
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v15_scientific_integer
*源码：*
```pascal
program t15;
var x: real;
begin
  x := 1e10;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t15, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, 1e10, 4, 8
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v16_scientific_signed_exp
*源码：*
```pascal
program t16;
var x: real;
begin
  x := 2E-3;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t16, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, 2E-3, 4, 8
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v17_string_basic
*源码：*
```pascal
program t17;
begin
  write("hi");
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t17, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, write, 3, 3
Delimiter, (, 3, 8
String, "hi", 3, 9
Delimiter, ), 3, 13
Delimiter, ;, 3, 14
Keyword, end, 4, 1
Delimiter, ., 4, 4
```

=== v18_comment_nested
*源码：*
```pascal
program t18;
{ outer { inner } still outer }
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t18, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 3, 1
Keyword, end, 4, 1
Delimiter, ., 4, 4
```

=== v19_directive_basic
*源码：*
```pascal
program t19;
{$DEFINE DEBUG}
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t19, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 3, 1
Keyword, end, 4, 1
Delimiter, ., 4, 4
```

=== v20_real_no_leading_zero
*源码：*
```pascal
program t20;
var x: real;
begin
  x := .5;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t20, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, .5, 4, 8
Delimiter, ;, 4, 10
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v21_hex_dollar
*源码：*
```pascal
program t21;
var x: integer;
begin
  x := $FF;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t21, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, $FF, 4, 8
Delimiter, ;, 4, 11
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v22_hex_0x
*源码：*
```pascal
program t22;
var x: integer;
begin
  x := 0x1A;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t22, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, 0x1A, 4, 8
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== v23_string_empty
*源码：*
```pascal
program t23;
begin
  write("");
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t23, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, write, 3, 3
Delimiter, (, 3, 8
String, "", 3, 9
Delimiter, ), 3, 11
Delimiter, ;, 3, 12
Keyword, end, 4, 1
Delimiter, ., 4, 4
```

=== v24_string_escape
*源码：*
```pascal
program t24;
begin
  write("a\\n\\t\\r\\0\\\\");
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t24, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, write, 3, 3
Delimiter, (, 3, 8
String, "a\\n\\t\\r\\0\\\\", 3, 9
Delimiter, ), 3, 28
Delimiter, ;, 3, 29
Keyword, end, 4, 1
Delimiter, ., 4, 4
```

=== v25_hex_boundary
*源码：*
```pascal
program t25;
var a: integer;
begin
  a := 0x0;
  a := 0xFFFFFFFF;
  a := $0;
  a := $FFFF;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t25, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, a, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, a, 4, 3
Operator, :=, 4, 5
Number, 0x0, 4, 8
Delimiter, ;, 4, 11
Identifier, a, 5, 3
Operator, :=, 5, 5
Number, 0xFFFFFFFF, 5, 8
Delimiter, ;, 5, 18
Identifier, a, 6, 3
Operator, :=, 6, 5
Number, $0, 6, 8
Delimiter, ;, 6, 10
Identifier, a, 7, 3
Operator, :=, 7, 5
Number, $FFFF, 7, 8
Delimiter, ;, 7, 13
Keyword, end, 8, 1
Delimiter, ., 8, 4
```

=== v26_scientific_boundary
*源码：*
```pascal
program t26;
var x: real;
begin
  x := 1e308;
  x := 1e-309;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t26, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Number, 1e308, 4, 8
Delimiter, ;, 4, 13
Identifier, x, 5, 3
Operator, :=, 5, 5
Number, 1e-309, 5, 8
Delimiter, ;, 5, 14
Keyword, end, 6, 1
Delimiter, ., 6, 4
```

=== v27_comment_mixed_cases
*源码：*
```pascal
program t27;
{ { { } } } { } { }
{ "not a string" }
begin
  write("{ comment inside string }");
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t27, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 4, 1
Keyword, write, 5, 3
Delimiter, (, 5, 8
String, "{ comment inside string }", 5, 9
Delimiter, ), 5, 36
Delimiter, ;, 5, 37
Keyword, end, 6, 1
Delimiter, ., 6, 4
```

=== v28_directives_extended
*源码：*
```pascal
program t28;
{$IFDEF DEBUG}
{$UNDEF DEBUG}
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, t28, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 4, 1
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

== 非法用例（invalid/，共 17 个）

=== i01_illegal_character_at
*源码：*
```pascal
program i01;
begin
  @
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i01, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, end, 4, 1
Delimiter, ., 4, 4
```
*错误信息：*
```
Error at 3:3 - Lexical error: illegal character (lexeme='@')
```

=== i02_illegal_character_hash
*源码：*
```pascal
program i02;
begin
  #
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i02, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, end, 4, 1
Delimiter, ., 4, 4
```
*错误信息：*
```
Error at 3:3 - Lexical error: illegal character (lexeme='#')
```

=== i03_malformed_number_multiple_dots
*源码：*
```pascal
program i03;
var x: real;
begin
  x := 1.2.3;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i03, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Delimiter, ;, 4, 13
Keyword, end, 5, 1
Delimiter, ., 5, 4
```
*错误信息：*
```
Error at 4:8 - Lexical error: malformed number (lexeme='1.2.3')
```

=== i04_malformed_number_incomplete_exponent
*源码：*
```pascal
program i04;
var x: real;
begin
  x := 1e+;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i04, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Delimiter, ;, 4, 11
Keyword, end, 5, 1
Delimiter, ., 5, 4
```
*错误信息：*
```
Error at 4:8 - Lexical error: malformed number (lexeme='1e+')
```

=== i05_unterminated_char_literal
*源码：*
```pascal
program i05;
var c: char;
begin
  c := 'a;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i05, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, c, 2, 5
Delimiter, :, 2, 6
Keyword, char, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, c, 4, 3
Operator, :=, 4, 5
Keyword, end, 5, 1
Delimiter, ., 5, 4
```
*错误信息：*
```
Error at 4:8 - Lexical error: unterminated character literal (lexeme=''a;')
```

=== i06_char_literal_too_long
*源码：*
```pascal
program i06;
var c: char;
begin
  c := 'ab';
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i06, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, c, 2, 5
Delimiter, :, 2, 6
Keyword, char, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, c, 4, 3
Operator, :=, 4, 5
String, 'ab', 4, 8
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== i07_unterminated_comment_brace
*源码：*
```pascal
program i07;
{ this comment never closes
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i07, 1, 9
Delimiter, ;, 1, 12
```
*错误信息：*
```
Error at 2:1 - Lexical error: unterminated comment (lexeme='{')
```

=== i08_unterminated_string_literal
*源码：*
```pascal
program i08;
begin
  write("hi);
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i08, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 2, 1
Keyword, write, 3, 3
Delimiter, (, 3, 8
Keyword, end, 4, 1
Delimiter, ., 4, 4
```
*错误信息：*
```
Error at 3:9 - Lexical error: unterminated string literal (lexeme='"hi);')
```

=== i09_unterminated_directive
*源码：*
```pascal
program i09;
{$IFDEF DEBUG
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i09, 1, 9
Delimiter, ;, 1, 12
```
*错误信息：*
```
Error at 2:1 - Lexical error: unterminated directive (lexeme='{$')
```

=== i10_malformed_hex_number
*源码：*
```pascal
program i10;
var x: integer;
begin
  x := 0xG1;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i10, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```
*错误信息：*
```
Error at 4:8 - Lexical error: malformed number (lexeme='0xG1')
```

=== i11_malformed_hex_alpha
*源码：*
```pascal
program i11;
var x: integer;
begin
  x := $G;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i11, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Delimiter, ;, 4, 10
Keyword, end, 5, 1
Delimiter, ., 5, 4
```
*错误信息：*
```
Error at 4:8 - Lexical error: malformed number (lexeme='$G')
```

=== i12_invalid_scientific_incomplete
*源码：*
```pascal
program i12;
var x: real;
begin
  x := 1e;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i12, 1, 9
Delimiter, ;, 1, 12
Keyword, var, 2, 1
Identifier, x, 2, 5
Delimiter, :, 2, 6
Keyword, real, 2, 8
Delimiter, ;, 2, 12
Keyword, begin, 3, 1
Identifier, x, 4, 3
Operator, :=, 4, 5
Delimiter, ;, 4, 10
Keyword, end, 5, 1
Delimiter, ., 5, 4
```
*错误信息：*
```
Error at 4:8 - Lexical error: malformed number (lexeme='1e')
```

=== i13_unterminated_directive_extended
*源码：*
```pascal
program i13;
{$UNDEF DEBUG
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i13, 1, 9
Delimiter, ;, 1, 12
```
*错误信息：*
```
Error at 2:1 - Lexical error: unterminated directive (lexeme='{$')
```

=== i14_invalid_directive_syntax
*源码：*
```pascal
program i14;
{$BADFLAG DEBUG}
begin
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, i14, 1, 9
Delimiter, ;, 1, 12
Keyword, begin, 3, 1
Keyword, end, 4, 1
Delimiter, ., 4, 4
```
*错误信息：*
```
Error at 2:1 - Lexical error: invalid directive (lexeme='{$BADFLAG DEBUG}')
```

=== lexical_error
*源码：*
```pascal
program bad_lex;
var a: integer;
begin
  a := 1 @ 2;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, bad_lex, 1, 9
Delimiter, ;, 1, 16
Keyword, var, 2, 1
Identifier, a, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, a, 4, 3
Operator, :=, 4, 5
Number, 1, 4, 8
Number, 2, 4, 12
Delimiter, ;, 4, 13
Keyword, end, 5, 1
Delimiter, ., 5, 4
```
*错误信息：*
```
Error at 4:10 - Lexical error: illegal character (lexeme='@')
```

=== semantic_error
*源码：*
```pascal
program bad_sem;
var a: integer;
begin
  a := true;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, bad_sem, 1, 9
Delimiter, ;, 1, 16
Keyword, var, 2, 1
Identifier, a, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Delimiter, ;, 2, 15
Keyword, begin, 3, 1
Identifier, a, 4, 3
Operator, :=, 4, 5
Keyword, true, 4, 8
Delimiter, ;, 4, 12
Keyword, end, 5, 1
Delimiter, ., 5, 4
```

=== syntax_error
*源码：*
```pascal
program bad_syn;
var a: integer
begin
  a := 1;
end.
```
*Token输出：*
```csv
Keyword, program, 1, 1
Identifier, bad_syn, 1, 9
Delimiter, ;, 1, 16
Keyword, var, 2, 1
Identifier, a, 2, 5
Delimiter, :, 2, 6
Keyword, integer, 2, 8
Keyword, begin, 3, 1
Identifier, a, 4, 3
Operator, :=, 4, 5
Number, 1, 4, 8
Delimiter, ;, 4, 9
Keyword, end, 5, 1
Delimiter, ., 5, 4
```


#pagebreak()
