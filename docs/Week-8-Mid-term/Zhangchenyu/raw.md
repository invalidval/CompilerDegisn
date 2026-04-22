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

/* 紧凑页：解决单页内容溢出 */
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


<!-- #region 每个人负责自己的部分 -->
<!-- _class: transition -->
# 03
## 符号表和语义分析
### 汇报人：张宸宇

---

## 模块边界与职责

<div class="cols-4060">
<div>


- 输入：语法阶段构建的 AST `ProgramNode* root`
- 依赖：`SymbolTable` + `ErrorHandler`
- 输出：
  - 为 AST 节点补全 `dataType` 与 `symbolEntry`
  - 记录语义错误：重定义、未定义、类型不匹配等
- 与主流程关系：
  - `main.cpp` 中 `SemanticAnnotator::annotate(root)` 在代码生成前执行
  - `errorHandler.hasErrors()` 为真则直接终止编译



</div>
<div>

```text
Pascal-S 源码
   -> Parser
   -> AST
   -> SemanticAnnotator
      (读写 SymbolTable)
   -> Annotated AST
   -> CodeGenerator
```

```cpp
SymbolTable symbolTable;
semantic_register::preregisterBuiltins(symbolTable);
ErrorHandler errorHandler;
SemanticAnnotator annotator(symbolTable, errorHandler);
annotator.annotate(root);
```

<div class="box-note">

在项目中的文件：
`include/symbol_table.h` / `src/symbol_table.cpp`  
`include/semantic_annotator.h` / `src/semantic_annotator.cpp`

</div>


</div>
</div>

---


## 符号表数据结构（一）：作用域与条目托管

<div class="cols">
<div>

### 作用域栈与条目池

```cpp
class SymbolTable {
private:
  std::vector<std::unordered_map<std::string, const SymbolEntry*>> scopes_;
  std::vector<std::unique_ptr<SymbolEntry>> entryArena_;
};
```

- scopes_：每层作用域一个哈希表，支持嵌套与遮蔽
- entryArena_：所有符号条目集中托管，保证指针稳定

</div>
<div>

### 名称规范化

```cpp
static std::string normalizeName(const std::string& name);
```

- 所有符号名插入/查找前统一转小写，保证大小写不敏感
- 语法、语义、代码生成各阶段都依赖此规范

<div class="box-tip">
构造时自动创建全局作用域，无需手动初始化
</div>

</div>
</div>

---

## 符号表数据结构（二）：SymbolEntry 字段

<div class="cols">
<div>

### 基础与类型信息

```cpp
struct SymbolEntry {
    std::string name;
    SymbolKind kind;
    DataType type;
    int scopeLevel;
    // ...
};
```
- name：小写名
- kind：符号类别（变量/常量/过程/函数/参数）
- type：语义类型（整型/实型/布尔/字符/函数/过程）
- scopeLevel：定义时的作用域层级

</div>
<div>

### 数组、常量、参数扩展

- isArray：是否为数组
- arrayBounds：多维数组边界
- hasConstLiteral/constLiteralText：常量原始文本
- isStringLikeConst：区分字符/字符串常量
- params：过程/函数参数列表
- isVarParam：过程/函数本身是否为 var 参数

<div class="box-tip">
所有字段均在 include/symbol_table.h 明确声明，类型安全
</div>

</div>
</div>

---

## 符号表数据结构（三）：参数结构 ParamInfo

<div class="cols">
<div>

### 参数信息结构体

```cpp
struct ParamInfo {
    std::string name;
    DataType type;
    bool isVarParam;
};
```

- name：参数名
- type：参数类型
- isVarParam：是否为 var 参数（引用传递）

</div>
<div>

### 典型用途

- params 字段用于过程/函数的参数签名
- 语义分析阶段用于参数类型检查和调用匹配
- 支持多参数、混合值传递与引用传递

<div class="box-tip">
ParamInfo 结构与 SymbolEntry 解耦，便于参数独立扩展
</div>

</div>
</div>

---

## 符号表条目设计

<div class="cols">
<div>

### 基础字段

```cpp
struct SymbolEntry {
    std::string name;
    SymbolKind kind = SymbolKind::Variable;
    DataType type = DataType::Unknown;
    int scopeLevel = 0;
};
```

- `name`：统一保存小写名
- `kind`：符号种类
- `type`：语义类型
- `scopeLevel`：插入时记录所在层级

### 数组与常量扩展

- `isArray`：标记是否为数组变量
- `arrayBounds`：保存多维数组边界
- `hasConstLiteral`：记录是否存在可回收字面量文本
- `constLiteralText`：保存常量原始文本
- `isStringLikeConst`：区分字符常量和字符串常量

</div>
<div>

### 参数与调用信息

- `params`：过程和函数的形参列表
- `isVarParam`：标记过程或函数本身是否来自 `var` 参数
- `ParamInfo.name`：参数名
- `ParamInfo.type`：参数类型
- `ParamInfo.isVarParam`：该参数是否按引用传递

### 工厂方法

```cpp
static SymbolEntry makeVariable(...);
static SymbolEntry makeConstant(...);
static SymbolEntry makeProcedure(...);
static SymbolEntry makeFunction(...);
static SymbolEntry makeParameter(...);
```

工厂方法把构造逻辑集中到一处，避免语义阶段手工拼装字段。

</div>
</div>

---

## 作用域栈设计

<div class="cols-6040">
<div>

### 作用域组织

```cpp
std::vector<std::unordered_map<std::string, const SymbolEntry*>> scopes_;
```

- `scopes_[0]` 是全局作用域
- 每次进入子程序分析都会压入新作用域
- 离开子程序时弹出当前作用域
- 退出只影响名字可见性，不回收 `entryArena_` 中的对象

### 两个查找接口

- `lookup(name)`：从内层到外层依次查找
- `lookupCurrentScope(name)`：只查当前层

这两个接口配合使用，可以同时支持名称解析和当前层重定义检测。

</div>
<div>

### 作用域操作实现

```cpp
void enterScope();
void exitScope();
```

- `enterScope()` 直接在尾部添加空表
- `exitScope()` 仅在层数大于 1 时弹出
- 这样保证全局作用域始终存在

### 运行结果

```text
全局作用域
  -> 过程作用域
      -> 过程内部局部变量
```

内部层可以遮蔽外层，但不会修改外层内容。

</div>
</div>

---

## 插入与查找机制

<div class="cols">
<div>

### 插入流程

```cpp
bool insert(SymbolEntry entry) {
    entry.name = normalizeName(entry.name);
    auto& current = scopes_.back();
    if (current.find(entry.name) != current.end()) {
        return false;
    }
    entry.scopeLevel = currentScopeLevel();
    entryArena_.push_back(std::make_unique<SymbolEntry>(std::move(entry)));
    const SymbolEntry* stored = entryArena_.back().get();
    current[stored->name] = stored;
    return true;
}
```

- 先规范化名字
- 再检查当前层是否重名
- 成功后写入 `entryArena_`
- 最后把地址挂到当前作用域表

### 失败场景

- 变量重定义
- 参数重定义
- 过程或函数重定义
- 常量在同层重复声明

</div>
<div>

### 查找流程

```cpp
const SymbolEntry* lookup(const std::string& name) const;
```

- 先规范化名字
- 再从内向外遍历 `scopes_`
- 找到第一个匹配项就返回
- 找不到就返回空指针

### 设计结果

- 支持内层遮蔽外层
- 支持从任何语义节点访问最近的定义
- 支持语义阶段把 `symbolEntry` 直接挂到 AST 节点上
- 为代码生成阶段提供稳定元数据

</div>
</div>

---

## 符号表与语义阶段协作

<div class="cols-6040">
<div>

### 预置符号

- `SemanticAnnotator` 构造时插入 `true` 和 `false`
- `main.cpp` 在语义分析前预注册 `read` 和 `write`
- 这样内建符号在全局层始终可见

### 语义阶段对符号表的使用

- 声明时调用 `insert()`
- 引用时调用 `lookup()`
- 当前层检测时可调用 `lookupCurrentScope()`
- 节点注解后保存 `symbolEntry`

</div>
<div>

### 主流程中的位置

```cpp
SymbolTable symbolTable;
semantic_register::preregisterBuiltins(symbolTable);
ErrorHandler errorHandler;
SemanticAnnotator annotator(symbolTable, errorHandler);
annotator.annotate(root);
```

### 这一层的实际作用

- 为变量、常量、参数建立唯一来源
- 为过程和函数调用提供参数签名
- 为数组访问提供边界信息
- 为 AST 节点提供稳定的语义绑定

</div>
</div>

<div class="box-note">

这部分是整个语义分析的基础层，后面的类型检查、参数检查、数组越界检查都依赖它。

</div>

---

## 语义分析详细设计

<div class="cols-6040">
<div>

### `SemanticAnnotator` 运行时上下文

- `functionContextStack_`
  - 跟踪当前函数，支持函数名左值赋值即返回值赋值
- `valueContextDepth_`
  - 区分值上下文，禁止把过程当作值使用
- `loopDepth_`
  - 约束 `break` 只能出现在 while/for 内

### 注解入口与分发

```cpp
void annotate(ASTNode* root) {
  annotateNode(root);
}

switch(node->nodeType) {
  case NodeType::VarDecl: annotateVarDecl(...); break;
  case NodeType::AssignStmt: annotateAssignStmt(...); break;
  case NodeType::BinaryExpr: annotateBinaryExpr(...); break;
  ...
}
```

</div>
<div>

### 声明阶段核心策略

- `ConstDecl`
  - 先注解右值，再推断类型并插入符号
  - 保存字面量文本，供后续阶段使用
- `VarDecl/ParamDecl`
  - 从类型节点推断 `DataType`
  - 批量处理标识符列表
- `ProcDecl/FuncDecl`
  - 先插入过程或函数签名，再 `enterScope()` 分析体
  - 退出时 `exitScope()`

<div class="box-note">
构造函数预置 `true/false` 布尔常量；
主流程还会预注册内建过程 `read/write`。
</div>

</div>
</div>

---

<!-- _class: compact -->
## 类型系统与语句检查规则

| 类别 | 规则 | 实现要点 |
|---|---|---|
| 赋值 | `lhs := rhs` | `isLValue(lhs)`；`isAssignable(lhsType, rhsType)` |
| 宽化转换 | integer -> real | 仅允许该方向隐式转换 |
| 条件 | if/while 条件必须 boolean | 否则 `reportTypeMismatch` |
| for 循环 | 控制变量、上下界必须 integer | 非法时报错 |
| break | 仅循环内部可用 | `loopDepth_ <= 0` 报错 |
| 过程调用 | 过程不能出现在值上下文 | `isValueContext()` 检查 |
| 表达式算子 | `+ - * / div mod and or not` | 分别做数值、整型、布尔约束 |

```cpp
bool isAssignable(DataType lhs, DataType rhs) const {
  if (lhs == DataType::Unknown || rhs == DataType::Unknown) return false;
  if (lhs == DataType::Real && rhs == DataType::Integer) return true;
  return lhs == rhs;
}
```

---

<!-- _class: mechanism-tight -->
## 类型系统与语句检查规则

| 类别 | 规则 | 实现要点 |
|---|---|---|
| 比较算子 | `= <> < <= > >=` | 两侧需互相可赋值 |
| 一元算子 | `not - uminus` | `not` 约束布尔；`-` 约束数值 |
| 可赋值判定 | 标识符与数组访问 | `Identifier.isLValue` 或 `ArrayAccess` |
| 函数结果赋值 | `f := expr` | 由 `functionContextStack_` 判定 |
| 调用形态 | 过程与函数 | 禁止过程出现在值上下文 |

---

<!-- _class: mechanism-tight -->
## 参数校验与数组语义

<div class="cols">
<div>

### 调用参数校验 `checkCallArguments`

- 参数个数：`expected != actual` 报错
- 参数类型：逐个 `isAssignable(param.type, actualType)`
- `var` 参数：实参必须是可赋值左值
- 内建过程特判：
  - `read(...)`：每个参数都必须可赋值
  - `write(...)`：只要求表达式可求值

</div>
<div>

### 数组边界机制

- 声明阶段：`collectArrayBounds()` 递归提取多维范围
- 访问阶段：
  - 下标类型必须 integer
  - 若下标可编译期求值 `tryEvalIntConst`，执行越界检查
  - 通过 `arrayAccessDepth()` 对齐当前维度边界

```text
a[i][j]
depth=0 -> 检查第1维
depth=1 -> 检查第2维
```

</div>
</div>

<div class="box-tip">
当前函数为 `f` 时，`f := expr` 视为合法左值赋值。
</div>

---

## 本模块错误覆盖与可验证点

### 语义错误覆盖

- 重定义：变量/常量/参数/过程/函数
- 未定义：标识符、被调用过程/函数
- 类型不匹配：赋值、条件、实参类型
- 调用错误：参数个数错误、`var` 参数传右值
- 控制流错误：循环外 `break`
- 数组错误：非数组下标、下标类型错误、可判定越界

### 测试与汇报建议展示

1. 选 2 个通过样例，展示注解后 AST 中 `dataType/symbolEntry` 生效。
2. 选 2 个失败样例，展示错误位置与报错文案。
3. 展示同名遮蔽与作用域退出后查找回退的对比样例。

<div class="note">
内容来源：严格依据当前仓库实现；`docs/详细设计.md` 仅用于章节组织与术语对齐。
</div>