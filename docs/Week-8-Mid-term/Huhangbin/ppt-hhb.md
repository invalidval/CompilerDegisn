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
  line-height: 1.32;
  box-sizing: border-box;
  padding: 56px 56px 92px 56px;
}
h1 { font-size: 40px; color: #1a237e; margin: 0 0 0.22em 0; }
h2 { font-size: 28px; color: #283593; border-bottom: 2px solid #3949ab; padding-bottom: 0.2em; margin: 0 0 0.48em 0; }

header, footer {
  color: var(--header-footer-color);
  font-size: 15px;
}

header { top: 14px; }
footer { bottom: 16px; }

ul, ol { margin: 0.2em 0 0.2em 1.1em; }
p { margin: 0.2em 0; }
pre { margin: 0.35em 0; }

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
pre, code { font-size: 17px; }

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

</style>
<!-- #endregion -->

<!-- _paginate: false -->
<!-- _header: "" -->
<!-- _footer: "" -->


<!-- #region 每个人负责自己的部分 -->
<!-- _class: transition -->
# 01
## 词法分析模块
### 汇报人：胡航宾

---

## 模块职责与交付边界

<style scoped>
.cols {
  grid-template-columns: 8fr 5fr;
  gap: 0.8em;
}
section {
  font-size: 18px;
  line-height: 1.22;
}
li {
  margin: 0.03em 0;
}
.box-tip, .box-note {
  padding: 0.42em 0.75em;
  margin: 0.18em 0;
}
.text-block {
  padding: 0.72em;
}
.boundary-note {
  font-size: 12px;
  color: #5f6f82;
  margin-top: 0.15em;
}
</style>

<div class="cols">
<div>

**模块职责（Lexer）**

- 将 Pascal-S 源码转为稳定 Token 流
- 维护 `line/column` 与 `yylloc` 位置信息
- 识别并收集词法错误，支持恢复扫描
- 为语法分析提供 `yylex()` 输入通道

<div class="box-tip">

结论：词法模块在主流程中已形成“输入-识别-定位-上报”的闭环能力。

</div>

**输入 / 输出（按实现）**

- 输入：`.pas` 字符流（`yyin`）
- 输出：Token 类型、词素、行列号、错误集合
- 调试输出：`--lex` 与 `--dump-tokens`

**实现约束**

- 扩展能力（字符串、十六进制、嵌套注释、指令）已文档化
- 参数约束：标识符最大长度 255，字符串最大长度 1024
- 与需求对齐：建议长度 8；工程实现放宽至 255

<div class="box-note">

依据：词法契约/详细设计（v1.1.0）+ `src/lexer.l`。

</div>

<div class="boundary-note">说明：该页聚焦职责；机制细节见后续“详细设计”页。</div>

</div>
<div>

<div class="text-block">

### 关键实现文件与接口

- `src/lexer.l`：词法规则、状态机、位置统计
- `src/main.cpp`：`runLexMode()` 与输出格式
- `include/error_handler.h`：统一错误结构

### 对外接口（示例）

- `lexerLastLexeme()/LastType()/LastLine()/LastColumn()`
- `lexerErrorCount()/lexerErrorAt(i)`
- `lexerResetState()/lexerClearErrors()`

</div>

<div class="note">本页回答：词法模块做什么、产出什么、如何被主流程消费。</div>

</div>
</div>

---

## 词法规则设计（需求到实现映射）

<div class="cols">
<div>

**核心规则集合**

1. 标识符：`[a-zA-Z_][a-zA-Z0-9_]*`
2. 关键字：大小写不敏感，归一化匹配
3. 数值：整数/实数/科学计数法/十六进制
4. 常量：字符常量、字符串常量（扩展）
5. 注释：`{...}` 跨行 + 嵌套（扩展），并兼容 `//` 单行注释
6. 指令：`{$...}` 独立状态机处理 + 白名单校验（扩展）

<div class="box-note">

关键词优先于标识符；多字符符号优先于单字符符号；非法指令显式报错。

</div>

</div>
<div>

**关键实现片段（简化）**

```cpp
IDENT   [a-zA-Z_][a-zA-Z0-9_]*
NUMBER  ([0-9]+|[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?)

{IDENT} {
  std::string lowered = toLower(yytext);
  if (isKeyword(lowered)) return keywordToken(lowered);
  yylval.text = strdup(lowered.c_str());
  return IDENTIFIER;
}

"{$" { directiveBody.clear(); BEGIN(DIRECTIVE); }
<DIRECTIVE>"}" {
  if (!isValidDirective(directiveBody)) reportLexError(..., "invalid directive", ...);
  BEGIN(INITIAL);
}
```

<div class = "text-block">
结论：词法规则覆盖需求分析 2.1 与 3.1 的必选能力，扩展项独立可控。
</div>

</div>

</div>

---

## 错误处理与恢复策略

<style scoped>
.cols3 { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 1.2em; }
.col-card { background: #f5f7ff; border-radius: 8px; padding: 1em; }
.col-card h3 { color: #3949ab; margin-top: 0; font-size: 20px; }
</style>

<div class="cols3">
<div class="col-card">

### 识别类错误

**典型类型**

- illegal character
- malformed number
- malformed character literal
- invalid directive
- identifier too long
- string literal too long
- numeric literal overflow

**策略**

报错后最小前进，继续扫描。

</div>
<div class="col-card">

### 未闭合类错误

**典型类型**

- unterminated comment
- unterminated string literal
- unterminated character literal
- unterminated directive

**策略**

在 EOF 处统一上报并安全结束。

</div>
<div class="col-card">

### 接口与输出

**对外接口**

- `lexerErrorCount()`
- `lexerErrorAt(i)`
- `lexerLastLine()/Column()`

**模式输出**

`--lex` 下稳定输出词法元信息。

`--dump-tokens` 下增加 Rule 字段与 token 统计。

</div>
</div>

<div class="note">错误协议：Lexical error: &lt;reason&gt; (lexeme='&lt;text&gt;')</div>

---

## 详细设计：状态机与行列号推进机制

<style scoped>
section { font-size: 20px; }
.cols { grid-template-columns: 3fr 2fr; gap: 1em; }
</style>

<div class="cols">
<div>

**状态机机制（与实现一一对应）**

1. `INITIAL`：识别标识符、关键字、数字、运算符、分隔符。
2. `COMMENT`：处理 `{...}` 注释并维护嵌套深度。
3. `DIRECTIVE`：处理 `{$...}` 指令并执行白名单校验。
4. `EOF`：集中处理未闭合结构并统一上报。

**位置统计关键点**

- 使用 `YY_USER_ACTION` 固化 token 起始 `line/column`。
- 每次匹配后统一推进当前位置，确保后续 token 定位不漂移。
- 注释与指令状态中同样持续推进行列号。

<div class="box-tip">

结论：错误定位精度来自“统一推进机制”，而非分散在各规则里的手工计数。

</div>

</div>
<div>

<img src="./Figs/lexer-state-machine.svg" style="width:100%; border-radius:8px;">

<div class="note">图示对应状态：INITIAL / COMMENT / DIRECTIVE / EOF。</div>

</div>
</div>

---

## 详细设计：错误恢复案例（可解释性）

<style scoped>
section { font-size: 20px; }
pre, code { font-size: 15px; line-height: 1.2; }
</style>

<div class="cols">
<div>

**恢复策略分层**

1. 识别类错误：最小前进，继续扫描。
2. 未闭合类错误：EOF 统一上报。
3. 指令类错误：保留扫描连续性，不阻断后续 token。

**典型样例与行为**

- `i01_illegal_character_at.pas`：报 `illegal character` 后继续。
- `i03_malformed_number_multiple_dots.pas`：报 `malformed number` 后继续。
- `i07_unterminated_comment_brace.pas`：EOF 报 `unterminated comment`。

</div>
<div>

**统一错误格式**

```text
Error at <line>:<column> -
Lexical error: <reason> (lexeme='<text>')
```

**恢复价值**

- 一次扫描尽可能收集更多错误
- 降低“修一个错再冒一个错”的迭代成本
- 为联调阶段提供稳定、可解释的失败信息

<div class="box-note">

该策略在 invalid 样例矩阵中已得到覆盖验证。

</div>

</div>
</div>

---

## 详细设计：观测与性能数据（dump模式）

<style scoped>
section { font-size: 20px; }
pre, code { font-size: 15px; line-height: 1.2; }
</style>

<div class="cols-6040">
<div>

**为什么需要 `--dump-tokens`**

1. 定位规则冲突：可看到每个 token 命中的规则标签。
2. 观察扫描规模：token 总数与字符串分配次数可量化。
3. 评估运行开销：输出耗时指标，支持回归对比。

```bash
./build/pascc -i test/cases/valid/v02_keyword_mixedcase.pas --dump-tokens
```

```text
Type, Lexeme, Line, Column, Rule
[dump] tokens=..., strdup_calls=..., elapsed_us=...
```

</div>
<div>

<div class="text-block">

### 观测结论

- 功能验证：`--lex`
- 规则验证：`--dump-tokens`
- 二者组合可形成“结果 + 机制”双证据链

</div>

<div class="box-tip">

词法阶段不仅可用，而且可解释、可度量、可回归。

</div>

</div>
</div>

---

## 测试覆盖与回归结果

<style scoped>
section { font-size: 20px; }
pre, code { font-size: 15px; line-height: 1.2; }
.slide5-note { font-size: 13px; color: #5f6f82; margin-top: 0.3em; }
.box-tip, .box-note { padding: 0.45em 0.8em; margin: 0.25em 0; }
</style>

<div class="cols-6040">
<div>

**测试矩阵（v1.7.0）**

1. valid 样例：28 个，28/28 通过
2. invalid 样例：14 个，14/14 按预期失败
3. 覆盖点：关键字、标识符、数字、字符/字符串、注释（含 //）、运算符、分隔符、错误恢复

**回归执行命令**

```bash
bash scripts/build.sh && bash test/run_tests.sh
bash test/test_lexer_unit.sh
./build/pascc -i test/cases/valid/v02_keyword_mixedcase.pas --dump-tokens
```

<div class="box-tip">

结果：构建通过、冒烟通过、矩阵通过、无失败样例。

</div>

<div class="box-note">

可观测性增强：--dump-tokens 额外输出 Rule、token 数、strdup 次数与耗时。

</div>

</div>
<div>

<div class="text-block">

### 数据结论

- 总样例数：42
- 通过率：100%
- 必选需求覆盖：100%
- 扩展能力验证：已完成

</div>

<div class="slide5-note">来源：词法测试矩阵-v1、词法完成度检验报告。</div>

</div>
</div>

---

## 阶段结论

<div class="cols">
<div>

**本阶段结论**

1. 需求分析中词法必选项已全部满足。
2. 契约、设计、实现、测试口径一致。
3. 词法层具备向 parser/semantic 联调的稳定输入能力。
4. 关键边界已在实现中落地（标识符 255、字符串 1024、数值溢出检测）。

<div class="box-tip">

词法模块当前状态：可验收、可联调、可回归。

</div>

</div>
<div>

**当前协作进展**

1. 后续语法、语义、代码生成工作已由组内成员推进。
2. 词法模块已稳定作为前端输入层参与联调。
3. 回归流程已纳入小组常态化测试链路。

<div class="box-note">

协作口径：词法侧持续维护契约与测试矩阵，保障团队并行开发一致性。

</div>

</div>
</div>

<!--- #endregion -->