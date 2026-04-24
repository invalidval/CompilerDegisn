# Pascal-S Record 类型扩展：参数与数组支持

**版本**：V1.0
**日期**：2026-04-24
**编写依据**：Record 类型参数和数组功能扩展实现
**前置文档**：[record类型实现设计.md](./record类型实现设计.md)

---

## 1. 引言

### 1.1 文档目的
本文档详细描述 Pascal-S 编译器中 Record 类型的功能扩展，在原有基础上新增支持：
- Record 类型作为函数/过程参数（值传递和引用传递）
- Record 类型作为数组元素类型
- Record 数组元素的字段访问（如 `people[i].age`）

### 1.2 扩展背景
原有实现已支持 Record 类型的基本功能（类型声明、变量声明、字段访问），但存在以下限制：
- 函数/过程参数只能是基本类型（integer, real, boolean, char）
- 数组元素只能是基本类型
- 无法实现复杂的数据结构操作（如学生信息数组、员工记录传递等）

### 1.3 设计目标
- **完整性**：支持 Record 类型在所有语法位置的使用
- **一致性**：参数和变量使用相同的类型信息处理逻辑
- **正确性**：正确处理混合访问（数组索引 + 字段访问）
- **兼容性**：不破坏现有功能，保持向后兼容

---

## 2. 核心问题分析

### 2.1 问题一：参数类型限制

**原始语法规则**：
```yacc
value_parameter:
    idlist ':' basic_type
```

**问题**：`basic_type` 只包含 INTEGER、REAL、BOOLEAN、CHAR，不支持用户定义类型。

**影响**：无法声明 Record 类型的参数，如：
```pascal
procedure printPerson(p: person);  // 编译失败
```

### 2.2 问题二：数组元素类型限制

**原始语法规则**：
```yacc
type:
    ARRAY '[' period ']' OF basic_type
```

**问题**：数组元素类型被限制为 `basic_type`。

**影响**：无法声明 Record 数组，如：
```pascal
var people: array[1..10] of person;  // 编译失败
```

### 2.3 问题三：混合访问解析

**语法挑战**：如何正确解析 `people[i].age`？

**问题分析**：
- `people[i]` 是数组访问，`i` 是变量（IdentifierNode）
- `.age` 是字段访问，`age` 也是标识符（IdentifierNode）
- 在 `id_varpart` 规则中，两者都被添加到同一个列表中
- 无法区分哪个是数组索引，哪个是字段名

**错误示例**：
```
Error: Record type 'person' has no field 'i'
```
编译器错误地将 `i` 当成了字段名。

---

## 3. 解决方案设计

### 3.1 语法层面修改

#### 3.1.1 参数类型规则扩展

**修改前**：
```yacc
value_parameter:
    idlist ':' basic_type
```

**修改后**：
```yacc
value_parameter:
    idlist ':' type
```

**效果**：参数类型从 `basic_type` 改为 `type`，支持任意类型（包括用户定义类型）。

#### 3.1.2 数组元素类型规则扩展

**修改前**：
```yacc
type:
    ARRAY '[' period ']' OF basic_type
```

**修改后**：
```yacc
type:
    ARRAY '[' period ']' OF type
```

**效果**：支持递归类型定义，数组元素可以是任意类型。

#### 3.1.3 混合访问解析策略

**核心思想**：使用节点类型来区分语义，而不是依赖节点内容。

**修改 `id_varpart` 规则**：
```yacc
id_varpart '.' IDENTIFIER
{
    ListNode* list = nullptr;
    if ($1 == nullptr) {
        list = g_astBuilder.makeList(ListKind::FieldAccess, currentPos());
    } else {
        list = asList($1);
        // 关键修改：改变列表的 kind 以标记混合访问
        list->kind = ListKind::FieldAccess;
    }

    // 关键修改：创建 FieldAccessNode 作为标记，而不是 IdentifierNode
    auto* fieldMarker = g_astBuilder.makeFieldAccess(nullptr, std::string($3), currentPos());
    list->add(fieldMarker);
    std::free($3);
    $$ = static_cast<void*>(list);
}
```

**设计要点**：
1. 当遇到 `.IDENTIFIER` 时，创建一个 FieldAccessNode（base 为 nullptr）作为标记
2. 这样在后续处理中，可以通过 `dynamic_cast<FieldAccessNode*>` 来识别字段名
3. 数组索引仍然是普通的表达式节点（IdentifierNode、LiteralNode 等）

**修改 `buildArrayAccessFromIndices` 函数**：
```cpp
ASTNode* buildArrayAccessFromIndices(ASTNode* base, ASTNode* indicesNode, SourcePos pos) {
    if (indicesNode == nullptr || indicesNode->nodeType != NodeType::List) {
        return base;
    }

    auto* list = static_cast<ListNode*>(indicesNode);

    // 遍历列表，根据节点类型决定操作
    ASTNode* current = base;
    for (ASTNode* child : list->children) {
        // 检查是否是字段访问标记（FieldAccessNode with nullptr base）
        if (auto* fieldAccess = dynamic_cast<FieldAccessNode*>(child)) {
            // 这是字段名，应用字段访问
            current = g_astBuilder.makeFieldAccess(current, fieldAccess->fieldName, pos);
        } else {
            // 这是数组索引，应用数组访问
            current = g_astBuilder.makeArrayAccess(current, child, 0, pos);
        }
    }
    return current;
}
```

**关键优势**：
- 通过节点类型（FieldAccessNode vs 其他）来区分语义
- 不依赖节点内容或属性（如 symbolEntry），在解析阶段就能正确区分
- 支持任意复杂的混合访问（如 `arr[i][j].field1.field2`）

---

## 4. 语义分析层面修改

### 4.1 类型推断增强

**问题**：`inferTypeFromTypeNode` 只识别基本类型关键字，不查找用户定义类型。

**修改前**：
```cpp
DataType SemanticAnnotator::inferTypeFromTypeNode(ASTNode* typeNode) const {
    if (auto* id = dynamic_cast<IdentifierNode*>(typeNode)) {
        const std::string typeName = toLower(id->identifier);
        if (typeName == "integer") return DataType::Integer;
        if (typeName == "real") return DataType::Real;
        // ... 其他基本类型 ...
    }
    return DataType::Unknown;
}
```

**修改后**：
```cpp
DataType SemanticAnnotator::inferTypeFromTypeNode(ASTNode* typeNode) const {
    if (auto* id = dynamic_cast<IdentifierNode*>(typeNode)) {
        const std::string typeName = toLower(id->identifier);

        // 先检查基本类型
        if (typeName == "integer") return DataType::Integer;
        if (typeName == "real") return DataType::Real;
        // ...

        // 新增：查找用户定义类型
        const SymbolEntry* typeSym = symbolTable_.lookup(typeName);
        if (typeSym && typeSym->kind == SymbolKind::TypeAlias) {
            return typeSym->type;
        }
    }
    return DataType::Unknown;
}
```

**效果**：支持从符号表中查找用户定义类型（如 `person`）。

### 4.2 参数声明增强

**问题**：`annotateParamDecl` 没有复制 Record 的字段信息，导致参数无法进行字段访问。

**核心思想**：参考 `annotateVarDecl` 的实现，确保参数和变量的类型信息一致性。

**修改后**：
```cpp
void SemanticAnnotator::annotateParamDecl(ParamDeclNode* node) {
    ASTNode* typeNode = node->children[1];
    DataType declaredType = inferTypeFromTypeNode(typeNode);
    const std::vector<ArrayBound> arrayBounds = collectArrayBounds(typeNode);

    // 新增：查找 Record 类型定义
    const SymbolEntry* typeSym = nullptr;
    std::string userTypeName;

    if (auto* typeId = dynamic_cast<IdentifierNode*>(typeNode)) {
        typeSym = symbolTable_.lookup(toLower(typeId->identifier));
        if (typeSym && typeSym->type == DataType::Record) {
            userTypeName = typeSym->typeName;
        }
    }

    // 声明参数时复制字段信息
    auto declareOne = [&](IdentifierNode* id) {
        SymbolEntry entry = SymbolEntry::makeParameter(id->identifier, declaredType, node->isVar);
        entry.typeName = userTypeName;  // 新增
        entry.fields = typeSym ? typeSym->fields : std::vector<FieldInfo>{};  // 新增
        entry.arrayBounds = arrayBounds;  // 新增
        symbolTable_.declare(entry);
        id->symbolEntry = symbolTable_.lookup(id->identifier);
    };

    // 处理参数列表
    // ...
}
```

**关键点**：
1. 查找类型定义，获取 `typeName` 和 `fields`
2. 将这些信息复制到参数的符号表条目中
3. 确保参数和变量具有相同的类型信息结构

### 4.3 变量声明增强

**问题**：数组变量的符号表条目中缺少数组边界信息。

**修改**：
```cpp
void SemanticAnnotator::annotateVarDecl(VarDeclNode* node) {
    // ... 已有逻辑 ...

    const std::vector<ArrayBound> arrayBounds = collectArrayBounds(typeNode);

    // 新增：保存数组边界信息
    entry.arrayBounds = arrayBounds;

    // ... 其他逻辑 ...
}
```

### 4.4 字段访问增强

**问题**：`annotateFieldAccess` 只处理直接标识符作为 base，不支持数组访问。

**修改后**：
```cpp
void SemanticAnnotator::annotateFieldAccess(FieldAccessNode* node) {
    annotateNode(node->children[0]);
    ASTNode* baseNode = node->children[0];

    const SymbolEntry* baseSym = nullptr;
    std::string baseTypeName;

    // 处理标识符
    if (auto* baseId = dynamic_cast<IdentifierNode*>(baseNode)) {
        baseSym = symbolTable_.lookup(baseId->identifier);
        if (baseSym && baseSym->type == DataType::Record) {
            baseTypeName = baseSym->typeName;
        }
    }
    // 新增：处理数组访问
    else if (auto* arrayAccess = dynamic_cast<ArrayAccessNode*>(baseNode)) {
        baseSym = arrayAccess->symbolEntry;
        if (baseSym && baseSym->type == DataType::Record) {
            baseTypeName = baseSym->typeName;
        }
    }

    // 查找 Record 类型定义
    const SymbolEntry* recordTypeSym = symbolTable_.lookup(baseTypeName);

    // 查找字段
    for (const auto& field : recordTypeSym->fields) {
        if (field.name == fieldName) {
            node->dataType = field.type;
            break;
        }
    }
}
```

**关键点**：
1. 支持 ArrayAccessNode 作为 base
2. 从 ArrayAccessNode 的 symbolEntry 中获取类型信息
3. 使用相同的字段查找逻辑

### 4.5 数组访问增强

**问题**：`annotateArrayAccess` 没有传递符号信息，导致后续字段访问失败。

**修改**：
```cpp
void SemanticAnnotator::annotateArrayAccess(ArrayAccessNode* node) {
    // ... 已有逻辑 ...

    // 新增：传递符号信息
    node->symbolEntry = base->symbolEntry;
    node->dataType = inferType(base);
}
```

---

## 5. 代码生成层面修改

### 5.1 参数类型生成

**问题**：`mapType` 函数只处理基本类型，对 Record 类型返回默认的 "int"。

**修改策略**：在调用 `mapType` 之前，先检查是否是用户定义类型。

**修改的函数**（共 4 个）：
1. `emitProcPrototype`
2. `emitProcDefinition`
3. `emitFuncPrototype`
4. `emitFuncDefinition`

**修改示例**：
```cpp
std::string CodegenUtils::emitProcPrototype(ProcDeclNode* node) {
    // ... 已有逻辑 ...

    for (ASTNode* param : params) {
        if (auto* paramDecl = dynamic_cast<ParamDeclNode*>(param)) {
            ListNode* idList = dynamic_cast<ListNode*>(paramDecl->children[0]);

            for (ASTNode* idNode : idList->children) {
                if (auto* id = dynamic_cast<IdentifierNode*>(idNode)) {
                    // 新增：检查是否是 Record 类型
                    std::string ctype;
                    if (id->symbolEntry &&
                        id->symbolEntry->type == DataType::Record &&
                        !id->symbolEntry->typeName.empty()) {
                        ctype = id->symbolEntry->typeName;  // 使用 Record 类型名
                    } else {
                        ctype = mapType(id->dataType);  // 使用基本类型映射
                    }

                    // 处理引用传递
                    if (paramDecl->isVar) {
                        ctype += "*";
                    }

                    paramList.push_back(ctype + " " + id->identifier);
                }
            }
        }
    }

    // ... 其他逻辑 ...
}
```

**关键点**：
1. 优先检查 `symbolEntry->typeName`
2. 如果是 Record 类型，直接使用类型名（如 "person"）
3. 否则使用 `mapType` 转换基本类型

---

## 6. 实现总结

### 6.1 修改文件清单

| 文件 | 修改内容 | 行数变化 |
|------|----------|----------|
| `src/parser.y` | 修改参数和数组类型规则，重写混合访问解析逻辑 | +30 |
| `src/semantic_annotator.cpp` | 增强类型推断、参数声明、字段访问、数组访问 | +50 |
| `src/codegen_utils.cpp` | 修改 4 个函数的参数类型生成逻辑 | +20 |

### 6.2 核心设计原则

1. **一致性原则**：参数和变量使用相同的类型信息处理逻辑
   - 参数声明参考变量声明的实现
   - 确保 `typeName`、`fields`、`arrayBounds` 信息完整传递

2. **信息传递原则**：在每个阶段保存足够的信息供下一阶段使用
   - 解析阶段：用 FieldAccessNode 标记字段名
   - 语义分析阶段：复制类型信息到符号表
   - 代码生成阶段：从符号表读取类型名

3. **类型区分原则**：用节点类型而非节点内容来区分语义
   - FieldAccessNode 标记字段名
   - 其他表达式节点表示数组索引
   - 通过 `dynamic_cast` 区分

4. **递归支持原则**：语法规则支持递归定义
   - `type` 可以包含 `type`
   - 支持任意嵌套的复杂类型

### 6.3 关键技术难点

**最难的部分**：处理 `people[i].age` 这种混合访问

**问题本质**：
- `i` 和 `age` 都是 IdentifierNode
- 在解析阶段无法通过节点内容区分
- 需要一种机制来标记语义差异

**解决方案**：
1. 在 `.IDENTIFIER` 分支中创建 FieldAccessNode（base 为 nullptr）作为标记
2. 在 `buildArrayAccessFromIndices` 中通过 `dynamic_cast` 识别标记
3. 根据节点类型决定是数组访问还是字段访问

**优势**：
- 不依赖语义信息（symbolEntry），在解析阶段就能正确处理
- 支持任意复杂的混合访问
- 代码清晰，易于维护

---

## 7. 测试与验证

### 7.1 测试用例

#### 7.1.1 Record 参数测试

**test_record_param_value.pas**（值传递）：
```pascal
program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;

function getAge(p: person): integer;
begin
  getAge := p.age
end;

begin
  p.age := 25;
  p.score := 95.5;
  write(getAge(p))
end.
```

**生成的 C 代码**：
```c
typedef struct {
    int age;
    float score;
} person;

person p;

int getAge(person p) {
    return p.age;
}

int main(void) {
    p.age = 25;
    p.score = 95.5;
    printf("%d", getAge(p));
    return 0;
}
```

**test_record_param_var.pas**（引用传递）：
```pascal
procedure modifyPerson(var p: person);
begin
  p.age := 30;
  p.score := 88.0
end;
```

**生成的 C 代码**：
```c
void modifyPerson(person* p) {
    p->age = 30;
    p->score = 88.0;
}
```

#### 7.1.2 Record 数组测试

**test_record_array_basic.pas**：
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

begin
  people[1].age := 20;
  people[1].score := 85.5;
  people[2].age := 25;
  people[2].score := 90.0;
  people[3].age := 30;
  people[3].score := 95.5;

  for i := 1 to 3 do
  begin
    write(people[i].age)
  end
end.
```

**生成的 C 代码**：
```c
typedef struct {
    int age;
    float score;
} person;

person people[3];
int i;

int main(void) {
    people[(1) - 1].age = 20;
    people[(1) - 1].score = 85.5;
    people[(2) - 1].age = 25;
    people[(2) - 1].score = 90.0;
    people[(3) - 1].age = 30;
    people[(3) - 1].score = 95.5;

    for (i = 1; i <= 3; i++) {
        printf("%d", people[(i) - 1].age);
    }

    return 0;
}
```

**运行结果**：`202530`（正确）

**test_record_array_sum.pas**（数组求和）：
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

**运行结果**：`75`（20 + 25 + 30 = 75，正确）

### 7.2 测试结果

| 测试用例 | 编译 | 运行 | 输出 | 状态 |
|---------|------|------|------|------|
| test_record_param_value.pas | ✅ | ✅ | 25 | 通过 |
| test_record_param_var.pas | ✅ | ✅ | 2530 | 通过 |
| test_record_param_multiple.pas | ✅ | ✅ | 100 | 通过 |
| test_record_array_basic.pas | ✅ | ✅ | 202530 | 通过 |
| test_record_array_sum.pas | ✅ | ✅ | 75 | 通过 |

### 7.3 验证要点

- ✅ Record 类型参数正确解析
- ✅ 值传递和引用传递语义正确
- ✅ Record 数组正确声明
- ✅ 混合访问（`arr[i].field`）正确解析
- ✅ 生成的 C 代码可编译
- ✅ 运行结果符合预期
- ✅ 不影响现有测试用例（向后兼容）

---

## 8. 与原实现的对比

### 8.1 功能对比

| 功能 | 原实现 | 扩展后 |
|------|--------|--------|
| Record 类型声明 | ✅ | ✅ |
| Record 变量声明 | ✅ | ✅ |
| 字段访问（`p.age`） | ✅ | ✅ |
| Record 作为参数 | ❌ | ✅ |
| Record 数组 | ❌ | ✅ |
| 混合访问（`arr[i].field`） | ❌ | ✅ |

### 8.2 修改思路对比

| 层面 | 原实现 | 扩展实现 | 修改思路 |
|------|--------|----------|----------|
| 语法 | 限制参数和数组为 `basic_type` | 改为 `type`，支持递归定义 | 放宽类型限制 |
| 解析 | 简单的字段访问 | 混合访问（数组+字段） | 用节点类型区分语义 |
| 语义 | 只处理变量的类型信息 | 参数和变量统一处理 | 一致性原则 |
| 代码生成 | 只映射基本类型 | 优先使用用户类型名 | 类型区分原则 |

### 8.3 设计优势

1. **完整性**：支持 Record 类型在所有语法位置的使用
2. **一致性**：参数、变量、数组元素使用统一的类型处理逻辑
3. **可扩展性**：递归类型定义支持未来的嵌套结构
4. **正确性**：通过节点类型区分语义，避免歧义
5. **兼容性**：不破坏现有功能，所有原有测试用例仍然通过

---

## 9. 未来扩展方向

### 9.1 潜在功能

- **嵌套 Record**：Record 字段本身也是 Record 类型
  ```pascal
  type
    address = record
      city: string;
      zip: integer
    end;
    person = record
      name: string;
      addr: address  // 嵌套 Record
    end;
  ```

- **Record 整体赋值**：支持 `p1 := p2`
  ```pascal
  var p1, p2: person;
  begin
    p1 := p2  // 整体赋值
  end.
  ```

- **Record 作为函数返回值**：
  ```pascal
  function createPerson(age: integer): person;
  begin
    createPerson.age := age;
    createPerson.score := 0.0
  end;
  ```

- **多维 Record 数组**：
  ```pascal
  var matrix: array[1..10, 1..10] of person;
  ```

### 9.2 优化方向

- **类型推导增强**：自动推导 Record 字段类型
- **错误恢复**：更友好的错误提示（如字段名拼写建议）
- **性能优化**：减少符号表查找次数，缓存类型信息

---

## 10. 总结

本次扩展成功实现了 Record 类型作为参数和数组元素的支持，关键创新点包括：

1. **节点类型标记法**：用 FieldAccessNode 标记字段名，解决混合访问解析难题
2. **一致性设计**：参数和变量使用相同的类型信息处理逻辑
3. **递归类型支持**：语法规则支持 `type` 递归定义，提高系统灵活性

修改涉及 3 个核心文件，新增约 100 行代码，所有测试用例通过，实现了预期的设计目标。

---

**文档结束**
