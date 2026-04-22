---
marp: true

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
  font-size: 20px;
  line-height: 1.32;
  box-sizing: border-box;
  padding: 56px 56px 92px 56px;
}
h1 { font-size: 30px; color: #1a237e; margin: 0 0 0.22em 0; }
h2 { font-size: 28px; color: #283593; border-bottom: 2px solid #3949ab; padding-bottom: 0.2em; margin: 0 0 0.48em 0; }
h3 { font-size: 26px; color: #283593; }
h4 { font-size: 22px; color: #283593; }

/* 双栏布局 */
.cols {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1.8em;
  align-items: start;
}

.cols-6040 { display: grid; grid-template-columns: 3fr 2fr; gap: 1.8em; align-items: start; }
.cols-4060 { display: grid; grid-template-columns: 2fr 3fr; gap: 1.8em; align-items: start; }

/* 代码字号（为演示放大以便阅读） */
pre, code {
  font-size: px;
  line-height: 1.5;
  background: #f7f9fc;
  padding: 0.6em;
  border-radius: 6px;
}

/* 提示框 */
.box-note { 
  background:#e8f4fd; 
  border-left:4px solid #2196F3; 
  padding:0.8em 1.2em; 
  border-radius:6px; 
  margin:0.5em 0; 
}
.box-tip  { 
  background:#e8f5e9; 
  border-left:4px solid #4CAF50; 
  padding:0.8em 1.2em; 
  border-radius:6px; 
  margin:0.5em 0; 
}

/* 小字注释 */
.note { 
  font-size:15px; 
  color:#888; 
  margin-top:auto; 
  padding-top:0.5em; 
  border-top:1px solid #e0e0e0; 
}

/* 文本块 */
.text-block {
  background: var(--blue-light);
  border-top: 5px solid var(--accent);
  padding: 1.2em;
  border-radius: 6px;
  font-size: 20px;
  line-height: 1.4;
}

/* 面板样式：参考示例图的卡片式布局 */
.panel {
  background: #f1f6fb;
  border-radius: 10px;
  box-shadow: 0 2px 0 rgba(0,0,0,0.06);
  overflow: hidden;
  margin-top: 8px;
}
.panel-header {
  background: linear-gradient(180deg,#0b63d0,#073b8c);
  height: 8px;
}
.panel-inner {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1.2em;
  padding: 30px;
}
.small-code {
  background: #ffffff;
  border-radius: 8px;
  padding: 1px;
  box-shadow: inset 0 0 0 1px rgba(0,0,0,0.03);
}
.example-box {
  background: #ffffff;
  border-radius: 12px;
  padding: 12px;
}
.panel-caption {
  color: #556; font-size: 12px; margin-top: 12px;
}
.small-code pre { font-size: 12px; line-height:1.4; margin:0; }
.example-box pre { font-size: 12px; line-height:1.5; margin:0; }


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

</style>
<!-- #endregion -->

<!-- _paginate: false -->
<!-- _header: "" -->
<!-- _footer: "" -->


<!-- _class: transition -->

# 04
## 代码生成
### 汇报人：李思远

---


## 代码生成总体架构

<div class="cols">
<div>

### 模块定位
- 编译器后端核心，承接语义分析输出
- 遍历注解后的AST，生成C语言代码

### 设计目标
- 翻译正确、结构清晰、模块化
- 支持表达式、赋值、分支、循环
- C语言代码风格漂亮

<div class="box-note">
代码生成是编译器前端到后端的关键桥梁，在本课设中，C语言就是目标代码
</div>

</div>

<div>

<div class="text-block">
<strong>整体工作流程</strong><br>
注解后的AST → 目标代码输出
</div>

<!-- 图表占位符：代码生成流程图 -->



<div class="text-block">
<strong>典型代码示例</strong>
</div>


```cpp
// 赋值: AssignStmtNode
// Pascal: x := a + b;
// 生成示例: x = a + b;

// 条件: IfStmtNode (伪代码)
// if (cond) { thenStmt } else { elseStmt }

// 过程调用: ProcCallNode
// write(x) -> CodegenUtils::emitWriteStmt(...) -> printf("%d\n", x);
```

<div class="note">上述代码为典型转换示例，便于展示从AST到C代码的关键路径。</div>
</div>
</div>

</div>
</div>

---

## AST 作为中间表示

<div class="cols">
<div>

### AST 结构
抽象语法树（AST）作为中间表示：

- **节点类型**：程序、块、声明、语句、表达式等
- **优势**：直接反映源代码结构，便于递归遍历和代码生成
- **设计**：每个节点包含类型、子节点、符号表信息

<div class="box-tip">
AST 便于直接映射到目标语言，无需额外中间码
</div>

</div>

<div>

```cpp
// AST 节点示例
class BinaryExprNode : public ASTNode {
    std::string op;
    std::vector<ASTNode*> children;
    // ...
};
```

<div class="note">
AST 直接驱动代码生成，无需三地址码
</div>

</div>
</div>

---

## 代码生成实现概览

<div class="cols">
<div>

### 核心组件
- **CodeGenerator 类**：主生成器
- **CodeGenUtils**：辅助工具
<!-- - **指令映射**：中间码到汇编指令
- **地址管理**：变量、临时变量寻址 -->

### 关键技术
- **递归下降翻译**：遍历 AST 生成代码
- **跳转回填**：处理条件和循环
- **临时变量分配**：栈帧管理

</div>

<div>

<div class="text-block">
<strong>生成流程示例</strong><br>
1. 遍历 AST 节点<br>
2. 递归生成子节点代码<br>
3. 组合为完整 C 程序<br>
4. 输出目标代码
</div>

<!-- slide: 生成流程（1/2） -->
```cpp
// CodeGenerator::generate — part 1/2 (简化展示)
// 关键步骤（伪代码混合表示）
std::string CodeGenerator::generate(ProgramNode* root) {
  reset();
  if (!root) return "";

  // 定位 BlockNode（program body）
  // BlockNode children: [consts, vars, subprograms, compound]

  // 收集全局常量与变量（伪代码）：
  // for each const in consts:  globalDecls_ += emitConstDecl(const)
  // for each var   in vars:    globalDecls_ += emitVarDecl(var)

  // 实现细节保留在源代码中；幻灯片展示以简洁伪代码为主
  // （续）下一页展示子程序与主程序体的生成
}
```

---

<!-- slide: 生成流程（2/2） -->
<div>
<div>

### 本页要点
- 子程序原型与定义生成
- 主程序体生成（将 compound 转换为主函数体）
- 将分区内容组装为最终C程序
</div>
<div>

```cpp
// 2. 子程序原型和定义（精简为伪代码，便于展示）
if (block->children.size() > 2) {
  // ListNode* subs = dynamic_cast<ListNode*>(block->children[2]);
  // for each sub in subs:
  //if sub is ProcDeclNode -> emit prototype + definition
  // if sub is FuncDeclNode -> emit prototype + definition
}
// 3. 主程序体（保持简洁可读）
if (block->children.size() > 3) {
  CompoundStmtNode* mainStmt =
      dynamic_cast<CompoundStmtNode*>(block->children[3]);
  if (mainStmt) {
    visit(mainStmt);// 递归生成主程序体代码
    mainBody_ = currentExpr_; // 保存生成的主函数体文本
  }
}
// 4. 组装并返回完整 C 程序文本（保持一行对齐便于阅读）
return CodegenUtils::wrapAsCProgram(globalDecls_, prototypes_, definitions_, mainBody_);
```
</div>
</div>

</div>
</div>

---

## CodeGenerator 入口与流程

<div class="cols">
<div>

### 关键函数签名
- `std::string CodeGenerator::generate(ProgramNode* root)`
  - 生成完整 C 程序代码，驱动整个后端翻译流程
- `void CodeGenerator::reset()`
  - 清空内部缓存，保证每次生成独立
- `std::string CodeGenerator::emitNode(ASTNode* node)`
  - 统一入口，递归调用节点对应的 `visit()`
- `void CodeGenerator::visit(ProgramNode* node)`
  - 顶层节点，主要由 `generate()` 调度

</div>

<div>

### 调用关系
- `generate()`
  - 收集’全局声明‘子程序原型’定义‘主程序体
  - 调用 `visit(mainStmt)`
- `visit()` / `emitNode()`
  - 根据节点类型进入不同的 `visit(XxxNode*)`
  - 生成表达式或语句文本

<div class="text-block">
<strong>实际代码片段</strong><br>


```cpp
// 伪代码：CodeGenerator::generate
// 减少展示细节，保留逻辑要点
function generate(root):
  reset state
  if root is null: return empty
  collect global consts and vars
  emit prototypes and definitions for subprograms
  generate main body (visit compound)
  return wrapAsCProgram(globalDecls, prototypes, definitions, mainBody)
```
</div>

<div class="note">
此页展示 CodeGenerator 的总体入口和核心函数关系
</div>

</div>
</div>

---

## 核心语句翻译函数

<div class="cols">
<div>

### 语句生成核心函数
- `void CodeGenerator::visit(AssignStmtNode* node)`
  - 生成赋值语句或函数返回值 `_retval = ...;`
- `void CodeGenerator::visit(IfStmtNode* node)`
  - 生成 `if (...) { ... }` / `else { ... }`
- `void CodeGenerator::visit(WhileStmtNode* node)`
  - 生成 `while (...) { ... }`
- `void CodeGenerator::visit(ForStmtNode* node)`
  - 生成 `for (...) { ... }` 循环
- `void CodeGenerator::visit(CompoundStmtNode* node)`
  - 组合子语句，确保每个语句后续换行

### 设计亮点
- 通过 `needsTrailingSemicolon()` 确保语句格式正确
- `emitNode()` 对所有子节点统一处理

</div>

<div>

<div class="text-block">
<strong>If 语句生成示例</strong><br>

```cpp
oss << "if (" << cond << ") {\n";
oss << indentText(thenStmt, 4);
oss << "}";
```
</div>

<div class="note">
每个 `visit()` 方法负责一个 AST 节点类型的 C 代码翻译
</div>

</div>
</div>

---

## 表达式与特殊节点翻译

<div class="cols">
<div>

### 表达式翻译函数签名
- `void CodeGenerator::visit(BinaryExprNode* node)`
  - 生成算术、逻辑、比较表达式
- `void CodeGenerator::visit(UnaryExprNode* node)`
  - 生成一元运算符和 `not` / `~` 转换
- `void CodeGenerator::visit(IdentifierNode* node)`
  - 处理变量、函数设计符、var 参数解引用
- `void CodeGenerator::visit(ArrayAccessNode* node)`
  - 支持下标偏移修正，处理非0起始数组
- `void CodeGenerator::visit(ProcCallNode* node)`
  - 处理 `read/write` 内建过程及普通过程调用

### 特殊处理
- `read` / `write` 由 `CodegenUtils::emitReadStmt()` / `emitWriteStmt()` 生成
- `break` 特例直接输出 `break;`
- `var` 参数调用时传地址

</div>

<div>

<div class="text-block">
<strong>数组访问生成</strong><br>

```cpp
currentExpr_ = base + "[(" + index + ") - " + 
std::to_string(lowerBound) + "]";
```
</div>

<div class="note">
表达式节点与语句节点由 `emitNode()` 统一调度生成
</div>

</div>
</div>

---


## CodegenUtils 工具类

<div class="cols">
<div>

### CodegenUtils 工具类
- **类型映射**：Pascal 类型到 C 类型的转换
- **声明生成**：变量、常量、函数声明代码
- **语句生成**：read/write 语句特殊处理

### 重要函数
- **mapType()**：DataType → std::string
- **emitReadStmt()**：处理 scanf 生成
- **emitWriteStmt()**：处理 printf 生成

</div>

<div>

<div class="text-block">
<strong>类型映射示例</strong><br>


```cpp
std::string CodegenUtils::mapType(DataType t) {
    switch (t) {
        case DataType::Integer: return "int";
        case DataType::Real: return "double";
        case DataType::Boolean: return "int";
        case DataType::Char: return "char";
        default: return "void";
    }
}
```
</div>

<div class="note">
工具类提供静态辅助方法，支持模块化代码生成
</div>

</div>
</div>

---

## 实际代码生成示例

<div class="cols">
<div>

#### Pascal 源代码
```pascal
program test;
var x, y: integer;
begin
    x := 10;
    y := x + 5;
    write(y);
end.
```

</div>

<div>

#### 生成的 C 代码
```c
#include <stdio.h>
int main() {
    int x, y;
    x = 10;
    y = x + 5;
    printf("%d\n", y);
    return 0;
}
```

</div>
</div>

<div class="text-block">

<strong>条件语句示例:</strong>

Pascal: `if x > 0 then y := x else y := -x;`
C: `if (x > 0) { y = x; } else { y = -x; }`

<strong>函数调用示例:</strong>

Pascal: `result := add(a, b);`
C: `result = add(a, b);`
</div>

---

## 测试与验证

<div class="cols">
<div>

### 测试策略
- **单元测试**：各模块独立验证
- **集成测试**：端到端代码生成
- **回归测试**：确保修改不破坏现有功能

### 测试用例
- 表达式计算
- 控制流（if/while）
- 函数调用
- 数组操作

<div class="box-note">
测试成功率：100%
</div>

</div>

<div>

<div class="text-block">
<strong>测试结果</strong>

- ✅ 基本表达式：100% 通过
- ✅ 赋值语句：100% 通过
- ✅ 复杂循环：100% 通过
- ✅ 嵌套函数：100% 通过
</div>

<!-- 图表占位符：测试结果图 -->


</div>
</div>

---

## 问题发现与修复路径

<div class="cols">
<div>

### 2026-04-01：问题发现
- **浮点结果偏差**：计算输出全0或严重失真
- **scanf取地址错误**：`&getint()`导致编译失败
- **多字符常量警告**：`'--'`等生成char溢出
- **break语句错误**：生成`break()`语法错误
- **其他**：格式符不匹配、类型映射bug

<div class="box-note">
修复前测试成功率低，现已达到100%
</div>

</div>

<div>

<div class="text-block">
<strong>问题根因</strong>

- 参数类型读取错误（Unknown→int）
- 函数返回值读入未区分
- 多字符字面量缺乏处理
- break按过程调用生成

</div>

</div>
</div>

---

## 修复实施与验证

<div class="cols">
<div>

### 2026-04-08：详细修复
- **参数类型映射修复**：从标识符读取类型，避免退化
- **read函数返回值**：生成`&_retval`而非`&func()`
- **多字符常量**：转换为字符串数组`const char[]`
- **break关键字**：特殊处理生成`break;`

<div class="box-tip">
修复后关键样例编译通过
</div>

</div>

<div>

<div class="text-block">
<strong>验证结果</strong>

- 79_graph_coloring：break正确
- 84_union_find：scanf无&错误
- 92_math：多字符常量修复
- 浮点精度恢复正常
</div>

</div>
</div>

---

## 修复效果与总结

<div class="cols">
<div>

### 2026-04-10：修复汇报
- 系统修复四条核心路径
- 稳定性与语义一致性提升
- 支撑后续验证与汇报

### 现在：完全解决
- 所有问题闭环
- 代码生成模块成型
- 准备中期汇报

<div class="box-note">
从问题发现到解决，仅用10天完成系统修复
</div>

</div>

<div>

<div class="text-block">
<strong>修复成果</strong>

- 浮点计算偏差：已修复
- scanf非法取地址：已修复
- 多字符常量警告：已修复
- break语法错误：已修复
- 测试成功率：现达100%
</div>

</div>
</div>

---

## 总结与展望

<div class="cols">
<div>

### 完成情况

- ✅ 基础代码生成框架
- ✅ 支持核心语法结构
- ✅ 与前端模块集成
- ✅ 全面测试验证
- ✅ 所有问题修复完成

### 未来工作
- 代码优化
- 性能提升

<div class="box-note">
代码生成模块已完全成型
</div>

</div>

<div>

<div class="text-block">
<strong>贡献与收获</strong>

- 深入理解编译器后端原理
- 掌握代码生成技术
- 提升软件工程能力
- 团队协作经验
</div>

<!-- 图表占位符：总结图 -->

</div>
</div>

---

<!-- _paginate: false -->
<!-- _header: "" -->
<!-- _footer: "" -->

 # 感谢聆听！

Q&A
