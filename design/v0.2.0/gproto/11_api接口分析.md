# 11 api 接口分析（psig v0.2.0）

> 前置：`08_功能设计.md`

## 接口细节

### 变更结构

```cpp
struct Table {
    std::vector<ColumnDef> columns;
    std::vector<TableRow> rows;
    std::string templateText;   // 新增：表级模板文本（多行，可空）
};

struct MatchResult {
    /* …既有字段… */
    std::string custom;         // 新增：自定义列文本（可空）
};

struct ResultExportOptions {    // 新增
    std::string templateText;            // 表首模板文本（可空）
    std::vector<std::string> columns;    // 结果列键（空=全部）
};
```

### 变更函数

| 函数 | 变更后签名 | 说明 |
| --- | --- | --- |
| ImportSignaturesCsv | `Table ImportSignaturesCsv(const std::string& path, std::string& err)` | 返回含 templateText |
| ImportSignaturesDb | `Table ImportSignaturesDb(const std::string& path, std::string& err)` | 同上 |
| ExportSignaturesCsv | `bool ExportSignaturesCsv(const std::string& path, const Table& t, std::string& err)` | 写 templateText |
| ExportSignaturesDb | `bool ExportSignaturesDb(const std::string& path, const Table& t, std::string& err)` | 写 sig_meta |
| ExportResultsCsv | `bool ExportResultsCsv(const std::string& path, const std::vector<MatchResult>& r, std::string& err, const ResultExportOptions& opt = {})` | +模板/选列/custom |
| ExportResultsDb | `bool ExportResultsDb(const std::string& path, const std::vector<MatchResult>& r, std::string& err, const ResultExportOptions& opt = {})` | 同上 |
| ExportResultsTxt | `bool ExportResultsTxt(const std::string& path, const std::vector<MatchResult>& r, std::string& err, const ResultExportOptions& opt = {})` | 同上 |

> 兼容性：`opt` 具默认值，既有 `ExportResultsXxx(path, results, err)` 调用保持可用。

## 约束满足说明

- 接口列表与细节已给出；无伪造 API。
