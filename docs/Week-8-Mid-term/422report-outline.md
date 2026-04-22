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

# Pascc 中期汇报

## 第八周-详细设计汇报

张宸宇 胡航宾 王嘉晗 李思远 谢康 · 2026.4.22

---
<!-- _class: toc -->
## 目录

1. [词法分析]()
2. [语法分析]()
3. [符号表和语义分析]()
4. [代码生成]()
<!-- 5. [错误处理与恢复]() -->

---
