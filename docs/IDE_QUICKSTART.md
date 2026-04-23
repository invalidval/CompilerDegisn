# PASCC IDE 快速开始

## 5 分钟上手指南

### 第 1 步：启动 IDE

```bash
cd /Users/zcy/Documents/3-2/Compiler/code
bash scripts/run_ide.sh test_ide_demo.pas
```

### 第 2 步：熟悉界面

启动后你会看到：

```
┌─────────────────────────────────────┬──────────────────┐
│     1  program test_ide;            │   编译输出       │
│     2  { 这是一个测试程序 }         │                  │
│     3  var                          │   Welcome to     │
│     4      x, y: integer;           │   PASCC IDE      │
│     5      result: integer;         │                  │
│     6                               │   Press F5 to    │
│     7  begin                        │   compile        │
│     8      x := 10;                 │                  │
│     9      y := 20;                 │                  │
│    10      result := x + y;         │                  │
│    11      write('Result: ');       │                  │
│    12      write(result)            │                  │
│    13  end.                         │                  │
└─────────────────────────────────────┴──────────────────┘
│ test_ide_demo.pas | Ln 1, Col 1 | F2:Save | F5:Compile│
└──────────────────────────────────────────────────────────┘
```

- **左侧**：代码编辑器（带行号和语法高亮）
- **右侧**：编译输出和运行结果
- **底部**：状态栏（显示文件名、光标位置、快捷键）

### 第 3 步：编译程序

按 **F5** 键编译。右侧输出面板会显示：

```
=== Compiling ===
File: test_ide_demo.pas
[OK] Compilation successful
```

### 第 4 步：运行程序

按 **F9** 键运行。输出面板会显示：

```
=== Running ===
=== Program Output ===
Result: 30
```

### 第 5 步：修改代码

1. 使用方向键移动光标到第 8 行
2. 修改 `x := 10;` 为 `x := 100;`
3. 按 **F2** 保存
4. 按 **F5** 重新编译
5. 按 **F9** 运行，查看新结果

## 常用操作

### 保存文件

**方式 1：F2 键（推荐）**
```
按 F2 → 文件已保存
```

**方式 2：命令模式**
```
按 Ctrl+X → 输入 w → 按 Enter
```

### 编译和运行

```
F5 → 编译（自动保存）
F9 → 运行编译后的程序
```

### 处理编译错误

如果编译出错：
1. 错误行会用**红色背景**高亮
2. 光标自动跳转到第一个错误
3. 右侧显示详细错误信息

示例：
```
=== Compiling ===
File: test.pas
Error at line 5: Undeclared identifier 'z'
[FAILED] Compilation failed
```

修复错误后，再次按 F5 重新编译。

### 跳转到指定行

```
按 Ctrl+G → 输入行号 → 按 Enter
```

### 退出 IDE

**方式 1：命令模式**
```
按 Ctrl+X → 输入 q → 按 Enter
```

**方式 2：快捷键**
```
按 Ctrl+Q
```

如果有未保存的更改，会提示你保存。

## 完整工作流示例

### 示例 1：创建新程序

```bash
# 1. 启动空白 IDE
bash scripts/run_ide.sh

# 2. 输入代码
program hello;
begin
    write('Hello, Pascal!')
end.

# 3. 保存（按 F2）
# 输入文件名: hello.pas

# 4. 编译（按 F5）
# 5. 运行（按 F9）
```

### 示例 2：调试错误程序

```bash
# 1. 打开文件
bash scripts/run_ide.sh test/my_program.pas

# 2. 编译（按 F5）
# 发现错误：Error at line 10: Undeclared identifier 'x'

# 3. IDE 自动跳转到第 10 行（红色高亮）

# 4. 修复错误：在第 10 行前添加变量声明
var x: integer;

# 5. 保存（按 F2）

# 6. 重新编译（按 F5）
# 编译成功！

# 7. 运行（按 F9）
```

### 示例 3：使用命令模式

```bash
# 1. 打开文件
bash scripts/run_ide.sh test.pas

# 2. 编辑代码...

# 3. 保存并退出
按 Ctrl+X → 输入 wq → 按 Enter
```

## 快捷键速查

| 按键 | 功能 |
|------|------|
| F2 | 保存文件 |
| F5 | 编译 |
| F9 | 运行 |
| Ctrl+X | 命令模式 |
| Ctrl+Q | 退出 |
| Ctrl+G | 跳转到行 |
| ↑↓←→ | 移动光标 |
| Home/End | 行首/行尾 |
| PgUp/PgDn | 翻页 |

## 命令模式速查

| 命令 | 功能 |
|------|------|
| :w | 保存 |
| :q | 退出 |
| :wq | 保存并退出 |
| :w filename | 另存为 |

## 提示和技巧

1. **F5 会自动保存**：编译前不需要手动保存
2. **错误自动跳转**：编译失败时会自动跳到第一个错误
3. **语法高亮**：关键字（蓝色）、注释（绿色）、字符串（黄色）
4. **当前行高亮**：正在编辑的行会反色显示
5. **使用启动脚本**：`run_ide.sh` 会自动配置终端环境

## 常见问题

**Q: Ctrl+S 不工作？**
- 使用 F2 代替，或确保通过 `run_ide.sh` 启动

**Q: 如何查看完整的编译输出？**
- 使用 + 和 - 键滚动输出面板

**Q: 如何打开其他文件？**
- 按 Ctrl+O，输入文件路径

**Q: 终端太小？**
- 调整终端窗口到至少 80x24

## 下一步

- 查看完整文档：`docs/IDE_GUIDE.md`
- 查看快捷键参考：`docs/IDE_SHORTCUTS.md`
- 尝试编译测试用例：`test/close_set/` 目录下的文件

## 反馈

如果遇到问题或有改进建议，欢迎提 Issue！

祝编程愉快！🚀
