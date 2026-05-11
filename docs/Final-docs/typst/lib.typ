#import "@preview/cuti:0.3.0": show-cn-fakebold

#show: show-cn-fakebold

#let experiment-report(
  row1: "",
  row2: "",
  lab: "",
  name: "",
  student-id: "",
  student-id1: "",
  student-id2: "",
  class: "",
  major: "",
  date: "",
  body,
) = {
  
  // 初始化相关页面、文本和段落样式
  set page(
    paper: "a4",
    margin: (top: 2.54cm, bottom: 2.54cm, left: 3.18cm, right: 3.18cm),
    footer: context {
    if counter(page).get().first() > 1 {
      align(center)[#counter(page).display("1")]
    }
    },
  )
  set text(
    font: ("Times New Roman", "SimSun"),//英文字体为Times New Roman，中文字体为宋体
    size: 12pt,
    weight: "regular",
    lang: "zh"//按中文方式排版断句
  )
  set par(first-line-indent: (amount:2em,all:true), leading: 1em)//首行缩进2字符，行距1倍
  set heading(numbering: "1.")//启用层级编号：1、1.1、1.1.1
  
  

  // 图表标题样式
  show figure.caption: it => {
    text(font: ("Times New Roman", "SimSun"), size: 9pt)[#it.body]
  }

  // 一级标题样式
  show heading.where(level: 1): it => [
    #align(left)[
      #par(first-line-indent: (amount:0em,all:true))[
        #text(font: ("Times New Roman", "Simsun"), size: 16pt, weight: "bold")[
          #if it.numbering != none {
            let nums = counter(heading).at(it.location())
            numbering("1. ", nums.at(0))
          }
          #it.body
        ]
      ]
    ]

  ]

  // 二级标题样式
  show heading.where(level: 2): it => [
    #align(left)[
      #par(first-line-indent: (amount:0em,all:true))[
        #text(font: ("Times New Roman", "Simsun"), size: 14pt, weight: "bold")[
          #if it.numbering != none {
            let nums = counter(heading).at(it.location())
            numbering("1.1 ", nums.at(0), nums.at(1))
          }
          #it.body
        ]
      ]
    ]
  ]

  // 三级标题样式
  show heading.where(level: 3): it => [
    #align(left)[
      #par(first-line-indent: (amount:0em,all:true))[
        #text(font: ("Times New Roman", "Simsun"), size: 12pt, weight: "bold")[
          #if it.numbering != none {
            let nums = counter(heading).at(it.location())
            numbering("1.1.1 ", nums.at(0), nums.at(1), nums.at(2))
          }
          #it.body
        ]
      ]
    ]
  ]

  // 代码标题样式
  let code-with-lines(code, lang: none) = {
    let lines = code.text.trim().split("\n")
    let numbered = lines.enumerate().map(it => {
      let (idx, line) = it
      let num = str(idx + 1)
      let padded = if num.len() < 2 { "0" + num } else { num }
      padded + "  " + line
    }).join("\n")
    raw(numbered, lang: lang)
  }

  show raw.where(block: true): elem => [
    #table(
      columns: (1fr,),
      [
        #block(width: 100%)[
          #text(size: 10pt)[
            #code-with-lines(elem, lang: elem.lang)
          ]
        ]
      ],
    )
  ]


  // 主体文档
  body
}

// 算法伪代码——三线表格式。steps 为字符串数组，通过 #step 直接插入，
// Typst 中字符串值作为表达式插入时原样输出，不会被解释为标记。
#let algorithm(alg-name, steps) = {
  let total = steps.len() + 1   // 总行数 = 步骤数 + 表头行
  let rows = ()

  // 表头行（跨两列）
  rows.push(table.cell(
    colspan: 2,
    fill: rgb("#e8edf2"),
  )[
    #text(font: ("Times New Roman", "SimSun"), size: 10.5pt, weight: "bold")[#alg-name]
  ])

  // 伪代码步骤行
  for (i, step) in steps.enumerate() {
    rows.push(table.cell(fill: rgb("#f8f9fa"))[
      #text(font: ("Consolas", "Courier New", "SimSun"), size: 10pt)[#str(i + 1)]
    ])
    rows.push(table.cell[
      #text(font: ("Consolas", "Courier New", "SimSun"), size: 10pt)[#step]
    ])
  }

  figure(
    kind: table,
    supplement: [算法],
    numbering: "1.1",
    caption: [算法伪代码： #alg-name],
    table(
      columns: (auto, 1fr),
      align: (x, y) => {
        if x == 0 and y > 0 { right }
        else { left }
      },
      stroke: (x, y) => {
        if y == 0 { (top: 1.5pt + black) }
        else if y == 1 { (bottom: 0.5pt + black) }
        else if y == total { (bottom: 1.5pt + black) }
      },
      inset: (x: 6pt, y: 4.5pt),
      ..rows,
    ),
  )
}

#let styled-parameter-table(cols: none, ..args) = {
  set table.cell(inset: (x: 6pt, y: 5pt))

  let default-cols = if cols != none {
    cols
  } else {
    (18%, 82%)
  }
  
  table(
    columns: default-cols,
    stroke: (x, y) => 0.5pt + rgb("#b7c3d0"),
    fill: (x, y) => {
      if y == 0 {
        rgb("#e6f0f9")
      } else if calc.odd(y) {
        white
      } else {
        rgb("#fcfdff")
      }
    },
    
    align: (x, y) => {
      if y == 0 or x == 0 {
        center + horizon
      } else {
        left + horizon
      }
    },
    ..args,
  )
}
