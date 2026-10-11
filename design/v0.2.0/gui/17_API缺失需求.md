# 17 API 缺失需求（v0.2.0）

> 前置：`16_API充分性审查.md`
> 目标：逐项列出静态库无法满足的能力、期望公共 API、输入输出、异常要求、验收标准。

## 缺失能力清单

### M-1：`Table` 表级模板文本 + 特征码 CSV 携带

- **期望公共 API**：`psig::Table` 增加字段 `std::string templateText;`；`ImportSignaturesCsv`/`ExportSignaturesCsv` 读写之。
- **输入**：文件路径；导出时 `Table{columns, rows, templateText}`。
- **输出**：导入时 `Table.templateText` 回填；导出文件包含模板文本块（置于文件**最前**）。
- **异常要求**：无模板标记的旧文件 → `templateText` 为空，不报错；文件不可读写 → 置 `err`。
- **验收标准**：导出含多行模板的 CSV → 重新导入 → `templateText` 逐字一致；旧 CSV（无模板）导入不报错。

### M-2：`Table` 表级模板文本 + 特征码 DB 携带

- **期望公共 API**：`ExportSignaturesDb`/`ImportSignaturesDb` 读写模板文本（DB 内新增元数据存储）。
- **输入/输出**：同 M-1。
- **异常要求**：旧库（无元数据表）→ 模板为空，不报错。
- **验收标准**：DB 往返模板一致；旧库兼容。

### M-3：`MatchResult` 自定义列字段

- **期望公共 API**：`psig::MatchResult` 增加字段 `std::string custom;`（自定义列文本）。
- **输入/输出**：作为结果导出数据源的一部分。
- **异常要求**：无（默认空串）。
- **验收标准**：`custom` 值可随结果导出。

### M-4：结果导出携带模板文本

- **期望公共 API**：结果导出 API 接受模板文本参数（如新增 `ResultExportOptions{ std::string templateText; }` 或等价出参）。
- **输入**：文件路径 + `results` + `templateText`。
- **输出**：导出文件包含模板（TXT 置于最前；CSV 前置注释块；DB 存入元数据）。
- **异常要求**：模板为空 → 正常导出。
- **验收标准**：导出文件可读回模板文本（TXT/CSV 至少可人工核验，DB 可程序读回）。

### M-5：结果导出选择列子集

- **期望公共 API**：结果导出 API 接受列选择参数（列键列表；空=全部）。
- **输入**：列键列表。
- **输出**：仅含选中列的导出文件。
- **异常要求**：空列表按「全部」处理；未知列键忽略或报错（静态库流程定）。
- **验收标准**：选择子集导出后，文件仅含选中列。

## 期望的 API 变更摘要（供静态库流程定稿）

```cpp
namespace psig {
struct Table { std::vector<ColumnDef> columns; std::vector<TableRow> rows;
               std::string templateText; };              // + templateText
struct MatchResult { /* …既有… */ std::string custom; };  // + custom

// 结果导出新增模板与列选择参数（形式由静态库流程定稿，示例）：
struct ResultExportOptions { std::string templateText; std::vector<std::string> columns; };
bool ExportResultsCsv(path, results, options, err);
bool ExportResultsDb (path, results, options, err);
bool ExportResultsTxt(path, results, options, err);
}
```

## 约束满足说明

- 逐项列出缺失能力、期望公共 API、输入输出、异常要求、验收标准（M-1..M-5）。
- 静态库完成契约返回前，GUI 停止 ViewModel 层相关编码（不得用模拟数据/假接口）。
