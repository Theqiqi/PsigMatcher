# 17 API 缺失需求文档

- 版本：v0.1.0
- 前置：`15_静态库存在性匹配.md`（API 充分性审查结论：静态库不存在，全部能力缺失）

> 逐项列出静态库无法满足的能力、期望公共 API、输入输出、异常要求、验收标准。

| 编号 | 缺失能力 | 期望公共 API | 输入 | 输出 | 异常要求 | 验收标准 |
| --- | --- | --- | --- | --- | --- | --- |
| M1 | 枚举进程 | `std::vector<ProcessInfo> psig::EnumerateProcesses()` | 无 | 进程列表 | 失败返回空/抛 PsigError | 列出系统进程含自身 |
| M2 | 解析模式串 | `Pattern psig::ParsePattern(const std::string&, const std::string&)` | 模式串,名称 | Pattern | 非法返回 valid=false | 合法/非法样例正确 |
| M3 | 批量扫描 | `std::vector<MatchResult> psig::ScanProcesses(const ScanRequest&, ProgressCallback, const std::atomic_bool*)` | 请求 | 命中列表 | 单进程失败跳过 | 已知命中可复现 |
| M4 | 取消扫描 | `void psig::RequestCancel(bool)` | 标志 | 无 | 线程安全 | 取消后尽快返回 |
| M5 | 导入特征码表 CSV | `Table psig::ImportSignaturesCsv(const std::string&, std::string&)` | 路径 | Table | 失败置 err | 往返一致 |
| M6 | 导入特征码表 DB | `Table psig::ImportSignaturesDb(const std::string&, std::string&)` | 路径 | Table | 非本 schema 置 err | 可读回导出库 |
| M7 | 导出特征码表 CSV/DB | `bool psig::ExportSignaturesCsv/Db(path, Table, err)` | 路径,Table | bool | 失败置 err | 文件可再导入 |
| M8 | 导出匹配结果 CSV/DB/TXT | `bool psig::ExportResultsCsv/Db/Txt(path, results, err)` | 路径,结果 | bool | 失败置 err | 三格式一致 |
| M9 | 动态列表格数据结构 | `ColumnDef` / `TableRow` / `Table` | — | — | — | 支撑自由列 |
| M10 | 并发安全 | 所有扫描/枚举 API 线程安全 | — | — | 无数据竞争 | 多线程调用稳定 |

## 约束满足说明

- 逐项列出缺失能力、期望公共 API、输入输出、异常要求、验收标准。
