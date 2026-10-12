# 11 api 接口分析（psig v0.2.4）

> 前置：`08_功能设计.md`

## 接口细节

### `std::vector<ProcessInfo> EnumerateProcesses();`
- 语义：枚举系统进程（pid/名称/路径/可访问）。
- 本次：名称来源修正（快照 `szExeFile` 优先、映像名兜底）。签名与返回结构不变。
- 异常：不抛异常；单个进程失败不影响整体。

### `bool ExportResultsTxt(path, results, err, options);`
- 语义：结果导出为 TXT。
- 本次：摘要行优先取 `options.summary`（非空），否则默认中文。
- 异常：写文件失败置 `err` 返回 false。

### `struct ResultExportOptions`
- 新增 `std::string summary;`（可空）。不影响 CSV/DB 行为。
