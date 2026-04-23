# Pascal-S Record 类型实现设计文档

**版本**：V1.0
**日期**：2026-04-23
**编写依据**：Record 类型功能实现代码

---

## 1. 引言

### 1.1 文档目的
本文档详细描述 Pascal-S 编译器中 Record 类型的实现方案，包括词法分析、语法分析、AST 扩展、符号表管理、语义分析和代码生成各阶段的设计与实现细节。

### 1.2 功能概述
Record 类型是 Pascal 语言中的结构化数据类型，允许将不同类型的数据组合成一个逻辑单元。本实现支持：
- Record 类型声明（`type` 关键字）
- Record 字段定义（支持多种基本类型）
- Record 变量声明
- 字段访问（点号语法：`record.field`）
- 代码生成为 C 语言的 `typedef struct`

### 1.3 设计原则
- **最小侵入性**：在现有编译器架构基础上扩展，不破坏已有功能
- **类型安全**：完整的类型检查和语义验证
- **C 语言映射**：Record 类型映射为 C 的 `typedef struct`，保证生成代码的可读性和兼容性
- **向后兼容**：不影响现有测试用例的通过率

---

## 2. 词法分析扩展

### 2.1 新增关键字
在 `src/lexer.l` 中新增以下关键字：

| 关键字 | Token | 用途 |
|--------|-------|------|
| `record` | RECORD | 标识 record 类型定义的开始 |
| `type` | TYPE | 标识类型声明部分 |
| `end` | END | 标识 record 类型定义的结束（复用已有 END token） |

### 2.2 词法规则
```lex
"record"    { return RECORD; }
"type"      { return TYPE; }
```

**说明**：
- 关键字统一转换为小写（Pascal-S 不区分大小写）
- `end` 关键字已存在，用于多种语法结构（begin-end、record-end）

---

## 3. AST 节点扩展

### 3.1 新增节点类型
在 `include/ast.h` 中新增 4 种 AST 节点：

#### 3.1.1 TypeDeclNode（类型声明节点）
```cpp
class TypeDeclNode : public ASTNode {
public:
    std::string name;  // 类型名称

    TypeDeclNode(const std::string& typeName, ASTNode* typeDef, SourcePos pos)
        : ASTNode(NodeType::TypeDecl, pos), name(typeName) {
        children.push_back(typeDef);  // children[0] = 类型定义
    }
};
```

**职责**：表示用户自定义类型声明（如 `type person = record ... end;`）

#### 3.1.2 RecordTypeNode（Record 类型节点）
```cpp
class RecordTypeNode : public ASTNode {
public:
    RecordTypeNode(ASTNode* fieldList, SourcePos pos)
        : ASTNode(NodeType::RecordType, pos) {
        children.push_back(fieldList);  // children[0] = 字段列表
    }
};
```

**职责**：表示 record 类型定义，包含字段列表

#### 3.1.3 FieldDeclNode（字段声明节点）
```cpp
class FieldDeclNode : public ASTNode {
public:
    FieldDeclNode(ASTNode* idList, ASTNode* typeNode, SourcePos pos)
        : ASTNode(NodeType::FieldDecl, pos) {
        children.push_back(idList);   // children[0] = 标识符列表
        children.push_back(typeNode); // children[1] = 类型节点
    }
};
```

**职责**：表示 record 中的字段声明（如 `age: integer;`）

#### 3.1.4 FieldAccessNode（字段访问节点）
```cpp
class FieldAccessNode : public ASTNode {
public:
    std::string fieldName;  // 字段名称

    FieldAccessNode(ASTNode* base, const std::string& field, SourcePos pos)
        : ASTNode(NodeType::FieldAccess, pos), fieldName(field) {
        children.push_back(base);  // children[0] = 基础表达式
    }
};
```

**职责**：表示字段访问表达式（如 `p.age`）

### 3.2 NodeType 枚举扩展
```cpp
enum class NodeType {
    // ... 已有类型 ...
    TypeDecl,      // 类型声明
    RecordType,    // Record 类型
    FieldDecl,     // 字段声明
    FieldAccess,   // 字段访问
};
```

---

## 4. 语法分析扩展

### 4.1 程序结构扩展
修改 `src/parser.y` 中的 `program_body` 规则，支持类型声明部分：

```yacc
program_body:
    const_declarations var_declarations subprogram_declarations compound_statement
  | const_declarations type_declarations var_declarations subprogram_declarations compound_statement
  | type_declarations var_declarations subprogram_declarations compound_statement
  | var_declarations subprogram_declarations compound_statement
  | subprogram_declarations compound_statement
  | compound_statement
  ;
```

**说明**：类型声明部分位于常量声明之后、变量声明之前

### 4.2 类型声明语法
```yacc
type_declarations:
    TYPE type_declaration_list
    ;

type_declaration_list:
    type_declaration
  | type_declaration_list type_declaration
  ;

type_declaration:
    IDENTIFIER '=' type_definition ';'
    {
        $$ = ASTBuilder::buildTypeDecl($1, $3, @1);
    }
    ;

type_definition:
    record_type
  | simple_type
  ;

record_type:
    RECORD field_list END
    {
        $$ = ASTBuilder::buildRecordType($2, @1);
    }
    ;

field_list:
    field_declaration
    {
        $$ = ASTBuilder::buildList({$1}, ListKind::FieldList, @1);
    }
  | field_list ';' field_declaration
    {
        $$ = ASTBuilder::appendToList($1, $3);
    }
  ;

field_declaration:
    id_list ':' type
    {
        $$ = ASTBuilder::buildFieldDecl($1, $3, @1);
    }
    ;
```

### 4.3 字段访问语法
修改 `variable` 规则，支持点号访问：

```yacc
variable:
    IDENTIFIER
  | IDENTIFIER '[' expression_list ']'
  | IDENTIFIER '.' IDENTIFIER
    {
        // 构建字段访问节点
        ASTNode* base = ASTBuilder::buildIdentifier($1, @1);
        $$ = ASTBuilder::buildFieldAccess(base, $3, @1);
    }
  | variable '.' IDENTIFIER
    {
        // 支持链式字段访问（如 a.b.c）
        $$ = ASTBuilder::buildFieldAccess($1, $3, @1);
    }
  ;
```

**关键点**：区分数组访问和字段访问
- 数组访问：`id[expr]`
- 字段访问：`id.field`

---

## 5. 类型系统扩展

### 5.1 DataType 枚举扩展
在 `include/common.h` 中扩展 `DataType` 枚举：

```cpp
enum class DataType {
    Integer,
    Real,
    Boolean,
    Char,
    Procedure,
    Function,
    Record,    // 新增：Record 类型
    Unknown
};
```

### 5.2 类型映射规则
| Pascal 类型 | DataType | C 类型 |
|-------------|----------|--------|
| `integer` | Integer | `int` |
| `real` | Real | `float` |
| `boolean` | Boolean | `int` |
| `char` | Char | `char` |
| `record` | Record | `typedef struct` |

---

## 6. 符号表扩展

### 6.1 SymbolKind 扩展
在 `include/symbol_table.h` 中新增符号种类：

```cpp
enum class SymbolKind {
    Variable,
    Constant,
    Procedure,
    Function,
    Parameter,
    TypeAlias  // 新增：类型别名（用户自定义类型）
};
```

### 6.2 SymbolEntry 扩展
扩展 `SymbolEntry` 结构以支持 Record 类型：

```cpp
struct SymbolEntry {
    std::string name;
    DataType type;
    SymbolKind kind;
    int level;

    // ... 已有字段 ...

    // Record 类型支持
    std::vector<ParamInfo> fields;  // 字段列表（复用 ParamInfo 结构）
    std::string typeName;           // 用户自定义类型名称

    // 工厂方法
    static SymbolEntry makeTypeAlias(const std::string& name, DataType type) {
        SymbolEntry entry;
        entry.name = name;
        entry.type = type;
        entry.kind = SymbolKind::TypeAlias;
        entry.level = 0;  // 类型定义通常在全局作用域
        return entry;
    }
};
```

**字段说明**：
- `fields`：存储 Record 的字段信息（字段名、类型）
- `typeName`：变量引用的用户自定义类型名称（如 `person`）

### 6.3 符号表操作
```cpp
// 插入类型别名
void SymbolTable::insert(const std::string& name, SymbolEntry entry) {
    std::string lowerName = toLower(name);
    currentScope_[lowerName] = entry;
}

// 查找类型定义
SymbolEntry* SymbolTable::lookup(const std::string& name) {
    std::string lowerName = toLower(name);
    // 先查找当前作用域，再查找外层作用域
    // ...
}
```

---

## 7. 语义分析扩展

### 7.1 新增语义分析方法
在 `include/semantic_annotator.h` 中声明：

```cpp
class SemanticAnnotator {
public:
    // ... 已有方法 ...

    void annotateTypeDecl(TypeDeclNode* node);
    void annotateRecordType(RecordTypeNode* node);
    void annotateFieldDecl(FieldDeclNode* node);
    void annotateFieldAccess(FieldAccessNode* node);
};
```

### 7.2 类型声明语义分析
```cpp
void SemanticAnnotator::annotateTypeDecl(TypeDeclNode* node) {
    std::string typeName = node->name;  // 从成员变量获取类型名

    // 检查类型名是否已存在
    if (symbolTable_->lookup(typeName) != nullptr) {
        errorHandler_->reportError(node->pos,
            "Type '" + typeName + "' already declared");
        return;
    }

    // 分析类型定义（RecordTypeNode）
    ASTNode* typeDef = node->children[0];
    annotateNode(typeDef);

    // 创建类型别名符号
    SymbolEntry typeEntry = SymbolEntry::makeTypeAlias(typeName, DataType::Record);

    // 收集字段信息
    if (RecordTypeNode* recordType = dynamic_cast<RecordTypeNode*>(typeDef)) {
        typeEntry.fields = collectFields(recordType);
    }

    // 插入符号表
    symbolTable_->insert(typeName, typeEntry);
}
```

### 7.3 字段收集逻辑
```cpp
std::vector<ParamInfo> SemanticAnnotator::collectFields(RecordTypeNode* node) {
    std::vector<ParamInfo> fields;
    ListNode* fieldList = dynamic_cast<ListNode*>(node->children[0]);

    if (!fieldList) return fields;

    // 遍历字段声明（处理嵌套 ListNode）
    for (ASTNode* child : fieldList->children) {
        if (ListNode* nestedList = dynamic_cast<ListNode*>(child)) {
            // 递归处理嵌套列表
            for (ASTNode* item : nestedList->children) {
                if (FieldDeclNode* fieldDecl = dynamic_cast<FieldDeclNode*>(item)) {
                    fields.push_back(extractFieldInfo(fieldDecl));
                }
            }
        } else if (FieldDeclNode* fieldDecl = dynamic_cast<FieldDeclNode*>(child)) {
            fields.push_back(extractFieldInfo(fieldDecl));
        }
    }

    return fields;
}
```

### 7.4 字段访问语义分析
```cpp
void SemanticAnnotator::annotateFieldAccess(FieldAccessNode* node) {
    // 分析基础表达式
    ASTNode* base = node->children[0];
    annotateNode(base);

    // 获取基础表达式的类型
    DataType baseType = base->dataType;
    if (baseType != DataType::Record) {
        errorHandler_->reportError(node->pos,
            "Field access requires record type");
        node->dataType = DataType::Unknown;
        return;
    }

    // 查找 Record 类型定义
    std::string typeName = base->symbolEntry ? base->symbolEntry->typeName : "";
    SymbolEntry* typeEntry = symbolTable_->lookup(typeName);

    if (!typeEntry || typeEntry->kind != SymbolKind::TypeAlias) {
        errorHandler_->reportError(node->pos,
            "Cannot resolve record type");
        node->dataType = DataType::Unknown;
        return;
    }

    // 查找字段
    std::string fieldName = node->fieldName;
    bool found = false;
    for (const auto& field : typeEntry->fields) {
        if (field.name == fieldName) {
            node->dataType = field.type;
            found = true;
            break;
        }
    }

    if (!found) {
        errorHandler_->reportError(node->pos,
            "Field '" + fieldName + "' not found in record");
        node->dataType = DataType::Unknown;
    }
}
```

### 7.5 变量声明语义分析扩展
```cpp
void SemanticAnnotator::annotateVarDecl(VarDeclNode* node) {
    // ... 已有逻辑 ...

    // 检查是否为用户自定义类型
    if (TypeIdentifierNode* typeId = dynamic_cast<TypeIdentifierNode*>(typeNode)) {
        std::string typeName = typeId->name;
        SymbolEntry* typeEntry = symbolTable_->lookup(typeName);

        if (typeEntry && typeEntry->kind == SymbolKind::TypeAlias) {
            // 使用用户自定义类型
            for (IdentifierNode* id : identifiers) {
                id->dataType = typeEntry->type;
                id->symbolEntry->typeName = typeName;
                id->symbolEntry->fields = typeEntry->fields;  // 复制字段信息
            }
        }
    }
}
```

### 7.6 左值检查扩展
```cpp
bool SemanticAnnotator::isLValue(ASTNode* node) {
    if (dynamic_cast<IdentifierNode*>(node)) return true;
    if (dynamic_cast<ArrayAccessNode*>(node)) return true;
    if (dynamic_cast<FieldAccessNode*>(node)) return true;  // 新增
    return false;
}
```

---

## 8. 代码生成扩展

### 8.1 生成结构调整
修改 `CodeGenerator::generate()` 方法，支持 5 段式结构：

```cpp
std::string CodeGenerator::generate(ProgramNode* root) {
    // 1. 常量声明
    // 2. 类型声明（typedef struct）
    // 3. 全局变量声明
    // 4. 函数原型和定义
    // 5. main 函数

    BlockNode* block = dynamic_cast<BlockNode*>(root->children[0]);
    if (!block) return "";

    // 分别处理各部分
    for (ASTNode* child : block->children) {
        if (child->nodeType == NodeType::TypeDecl) {
            visit(dynamic_cast<TypeDeclNode*>(child));
        }
        // ... 其他节点类型 ...
    }

    return CodegenUtils::wrapAsCProgram(
        globalDecls_, typeDecls_, prototypes_, definitions_, mainBody_
    );
}
```

### 8.2 类型声明代码生成
```cpp
void CodeGenerator::visit(TypeDeclNode* node) {
    std::ostringstream oss;

    // 生成 typedef struct
    oss << "typedef struct {\n";

    // 生成字段声明
    RecordTypeNode* recordType = dynamic_cast<RecordTypeNode*>(node->children[0]);
    if (recordType) {
        visit(recordType);
        oss << currentExpr_;
    }

    oss << "} " << node->name << ";\n";

    typeDecls_ += oss.str();
    currentExpr_ = "";
}
```

### 8.3 字段声明代码生成
```cpp
void CodeGenerator::visit(FieldDeclNode* node) {
    ListNode* idList = dynamic_cast<ListNode*>(node->children[0]);
    ASTNode* typeNode = node->children[1];

    for (ASTNode* idNode : idList->children) {
        IdentifierNode* id = dynamic_cast<IdentifierNode*>(idNode);
        if (!id) continue;

        std::string ctype = CodegenUtils::mapType(id->dataType);
        currentExpr_ += "    " + ctype + " " + id->identifier + ";\n";
    }
}
```

### 8.4 字段访问代码生成
```cpp
void CodeGenerator::visit(FieldAccessNode* node) {
    // 生成基础表达式
    visit(node->children[0]);
    std::string baseExpr = currentExpr_;

    // 生成字段访问：base.field
    currentExpr_ = baseExpr + "." + node->fieldName;
}
```

### 8.5 变量声明代码生成扩展
```cpp
std::string CodegenUtils::emitVarDecl(VarDeclNode* node) {
    // ... 已有逻辑 ...

    for (IdentifierNode* id : identifiers) {
        // 检查是否为 Record 类型
        if (id->symbolEntry &&
            id->symbolEntry->type == DataType::Record &&
            !id->symbolEntry->typeName.empty()) {
            // 使用用户自定义类型名
            oss << id->symbolEntry->typeName << " " << id->identifier << ";\n";
        } else {
            // 使用标准类型映射
            std::string ctype = mapType(id->dataType);
            oss << ctype << " " << id->identifier << ";\n";
        }
    }

    return oss.str();
}
```

---

## 9. 代码生成示例

### 9.1 基本示例
**Pascal-S 输入**：
```pascal
program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;
begin
  p.age := 25;
  p.score := 95.5;
  write(p.age);
  write(p.score)
end.
```

**生成的 C 代码**：
```c
#include <stdio.h>

typedef struct {
    int age;
    float score;
} person;

person p;

int main(void) {
    p.age = 25;
    p.score = 95.5;
    printf("%d", p.age);
    printf("%f", p.score);
    return 0;
}
```

### 9.2 多变量示例
**Pascal-S 输入**：
```pascal
program test;
type
  person = record
    age: integer;
    score: real;
    initial: char
  end;
var
  p1, p2: person;
begin
  p1.age := 25;
  p1.score := 95.5;
  p1.initial := 'A';

  p2.age := 30;
  p2.score := 88.0;
  p2.initial := 'B';

  write(p1.age);
  write(p1.score);
  write(p2.age);
  write(p2.score)
end.
```

**生成的 C 代码**：
```c
#include <stdio.h>

typedef struct {
    int age;
    float score;
    char initial;
} person;

person p1;
person p2;

int main(void) {
    p1.age = 25;
    p1.score = 95.5;
    p1.initial = 'A';
    p2.age = 30;
    p2.score = 88.0;
    p2.initial = 'B';
    printf("%d", p1.age);
    printf("%f", p1.score);
    printf("%d", p2.age);
    printf("%f", p2.score);
    return 0;
}
```

---

## 10. 测试与验证

### 10.1 测试用例
创建以下测试文件验证功能：

1. **test_record_basic.pas**：基本 Record 声明和字段访问
2. **test_record_multi_field.pas**：多字段 Record
3. **test_record_multi_var.pas**：多个 Record 变量
4. **test_record_comprehensive.pas**：综合测试

### 10.2 测试方法
```bash
# 编译测试文件
./build/pascc -i test_record_basic.pas -o output.c

# 编译生成的 C 代码
gcc output.c -o test

# 运行测试
./test
```

### 10.3 验证要点
- ✅ 类型声明正确解析
- ✅ 字段访问类型推导正确
- ✅ 生成的 C 代码可编译
- ✅ 运行结果符合预期
- ✅ 不影响现有测试用例（向后兼容）

---

## 11. 实现总结

### 11.1 修改文件清单
| 文件 | 修改内容 |
|------|----------|
| `src/lexer.l` | 新增 `record`, `type` 关键字 |
| `include/ast.h` | 新增 4 种 AST 节点类型 |
| `src/ast.cpp` | 实现 AST 节点构建方法 |
| `src/parser.y` | 扩展语法规则支持类型声明和字段访问 |
| `include/common.h` | 扩展 `DataType` 枚举 |
| `include/symbol_table.h` | 扩展 `SymbolKind` 和 `SymbolEntry` |
| `src/symbol_table.cpp` | 实现类型别名相关方法 |
| `include/semantic_annotator.h` | 声明 Record 相关语义分析方法 |
| `src/semantic_annotator.cpp` | 实现类型检查和字段验证逻辑 |
| `src/code_generator.cpp` | 实现 Record 代码生成 |
| `src/codegen_utils.cpp` | 扩展变量声明生成逻辑 |

### 11.2 关键技术点
1. **AST 节点设计**：字段名存储在节点成员变量中，而非子节点
2. **字段收集**：处理嵌套 `ListNode` 结构
3. **类型传播**：通过 `typeName` 和 `fields` 在符号表中传递类型信息
4. **左值识别**：`FieldAccessNode` 可作为赋值语句左值
5. **代码生成顺序**：类型声明必须在变量声明之前

### 11.3 设计优势
- **类型安全**：完整的编译期类型检查
- **可扩展性**：易于支持嵌套 Record、Record 数组等高级特性
- **代码质量**：生成的 C 代码清晰、符合 C 语言规范
- **向后兼容**：不影响现有功能和测试用例

---

## 12. 未来扩展方向

### 12.1 潜在功能
- **嵌套 Record**：Record 字段本身也是 Record 类型
- **Record 数组**：`array of record`
- **Record 参数传递**：过程/函数参数支持 Record 类型
- **Record 赋值**：整体赋值（`p1 := p2`）

### 12.2 优化方向
- **内存布局优化**：字段对齐和填充
- **类型推导增强**：更智能的类型推导
- **错误恢复**：更友好的错误提示

---

**文档结束**
