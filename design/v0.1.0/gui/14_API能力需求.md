# 14 API 能力需求文档

- 版本：v0.1.0
- 前置：`11_GUI执行路径.md`、静态库 API 文档（暂无）

> 本文档逐条列出 GUI 所需的静态库 API 能力、期望公共 API、输入输出、异常要求、验收标准。
> 所有能力必须由静态库公共 API 提供，GUI 不得用 OS API 自行实现。

## 能力清单

### C1 枚举进程
- 用途：进程选择页列出可扫描进程。
- 期望 API：`std::vector<ProcessInfo> psig::EnumerateProcesses()`
- 输入：无。输出：进程列表（pid、名称、路径、是否可访问）。
- 异常要求：失败返回空列表或抛 `psig::PsigError`；线程安全。
- 验收：在 Windows 上返回当前系统进程，含本进程。

### C2 解析/校验特征码模式串
- 用途：校验用户输入模式串、把模式串转为字节+掩码。
- 期望 API：`Pattern psig::ParsePattern(const std::string& raw, const std::string& name)`
- 输入：模式串（如 `48 8B ?? 05`）、名称。输出：`Pattern{bytes,mask,valid,error}`。
- 异常要求：非法串返回 `valid=false` + `error`，不抛异常。
- 验收：合法串解析正确；`??` 通配；非法 token 报错。

### C3 批量扫描
- 用途：对（多进程 × 多特征码）批量匹配。
- 期望 API：
  `std::vector<MatchResult> psig::ScanProcesses(const ScanRequest& req, ProgressCallback cb, const std::atomic_bool* cancel)`
- 输入：目标 pid 列表、特征码列表、每模式上限。输出：命中列表 + 进度回调。
- 异常要求：单进程读取失败跳过并记录，不中断整体；线程安全；支持取消。
- 验收：对已知进程已知模式命中；取消能尽快返回。

### C4 取消扫描
- 用途：用户中途取消。
- 期望 API：`void psig::RequestCancel(bool flag)` 或经 `ScanRequest.cancel` 原子标志。
- 验收：置位后后台尽快停止并返回已收集结果。

### C5 导入特征码表（CSV）
- 期望 API：`Table psig::ImportSignaturesCsv(const std::string& path, std::string& err)`
- 输入：CSV 路径。输出：`Table{columns,rows}`。异常：失败置 err 并返回空表。
- 验收：往返导入导出一致。

### C6 导入特征码表（SQLite .db）
- 期望 API：`Table psig::ImportSignaturesDb(const std::string& path, std::string& err)`
- 输入：`.db` 路径。输出：Table。异常：非本程序 schema → err。
- 验收：能读回 C7 导出的库。

### C7 导出特征码表（CSV / DB）
- 期望 API：`bool psig::ExportSignaturesCsv(path, Table, err)` / `bool psig::ExportSignaturesDb(path, Table, err)`
- 验收：文件生成且可再导入。

### C8 导出匹配结果（CSV / DB / TXT）
- 期望 API：`bool psig::ExportResultsCsv(path, results, err)` / `...Db...` / `...Txt...`
- 验收：三种格式文件生成，内容与结果集一致。

## 数据模型能力（供 GUI 显示）

- `ProcessInfo`：pid/name/path/accessible。
- `Pattern`：name/raw/bytes/mask/valid/error。
- `MatchResult`：patternName/pid/processName/address/moduleName/moduleBase/matchedBytes/hitTime。
- `ColumnDef`：key/title/type/builtin（支撑自由列）。
- `Table`/`TableRow`：动态列表格（支撑导入导出）。

## 并发要求

- 所有扫描/枚举 API 必须线程安全，可被 QThread/QtConcurrent 调用。
- 进度回调在后台线程触发，GUI 侧经 QueuedConnection 回主线程。

## 约束满足说明

- 逐条列出能力、期望 API、输入输出、异常要求、验收标准。
- 全部能力指向静态库公共 API，未用 OS API 作为 GUI 能力提供方。
