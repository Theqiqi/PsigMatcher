# 14 API 能力需求分析（v0.2.4，增量）

- 模式：修改型。聚焦 v0.2.4 改动点新增/变更的静态库能力需求。
- 既有静态库：`psig`（公开 API 见 `psig/include/psig.h`）。

## 能力需求清单

### R-01 进程名来源修正（C-01）
- **期望能力**：`psig::EnumerateProcesses()` 返回的 `ProcessInfo::name` 对**所有**进程均非空且为真实进程名（与任务管理器一致），含受保护进程（Registry / MemCompression / Secure System 等）。
- **期望公共 API**：**签名不变** —— `std::vector<ProcessInfo> EnumerateProcesses();`（行为修正）。
- **输入/输出**：无输入 → 进程列表（`name` 一律非空）。
- **异常要求**：个别进程取路径失败不影响名称；不抛异常。
- **验收标准**：任意枚举结果 `name` 非空；受保护进程名与任务管理器一致；`accessible` 语义不变。

### R-02 结果导出摘要随语言（C-03）
- **期望能力**：结果 TXT 导出的**摘要行**（现写死 `"匹配结果 (N 条)"`）可由调用方指定语言。
- **期望公共 API**：`psig::ResultExportOptions` 新增字段 `std::string summary;`（完整摘要文案，含数量，可空）。
- **行为**：`ExportResultsTxt` 若 `summary` 非空 → 以其替换默认摘要行；为空 → 回落现默认中文（向后兼容）。
- **输入/输出**：调用方传本地化摘要串 → TXT 摘要行为该串。
- **异常要求**：不改变行格式与其余内容。
- **验收标准**：英文 UI 传 `"Results (N)"` → TXT 摘要为英文；不传 → 中文（兼容）。

### 无需静态库变更的改动点
- **C-03 列头（`备注`）**：`ResultExportOptions.headers` 已存在；「备注」中文系 GUI 侧 `resultHeaderForKey("custom")` 硬编码，属 **GUI 修复**，无需 psig 变更。
- **C-04 特征码表解耦模板**：`Table::templateText` 保留；GUI 侧 `tableFromModel`/`applyTableToModel` 不再读写即达成。**无需 psig 变更**。
- **C-05 入口收敛**：GUI 文件对话框过滤器；psig 的 `ImportSignaturesCsv`/`ExportSignaturesCsv`/`ExportResultsDb` 保留。**无需 psig 变更**。
- **C-02 整行 hover**：纯 GUI（delegate/qss）。**无需 psig 变更**。

## 能力覆盖判定

| 需求 | 现有 psig API 是否覆盖 | 处置 |
| --- | --- | --- |
| R-01 进程名来源 | 否（行为不足） | 走静态库流程修正 `EnumerateProcesses`（原子层） |
| R-02 结果摘要随语言 | 否（无字段） | 走静态库流程扩展 `ResultExportOptions` |

- 均为**既有 API 的行为修正/字段扩展**，静态库 `psig` 已存在，无需新建库，但需走「静态库跨流程需求接收」（步骤 20）。
- GUI 不直接实现缺失功能、不调用平台 API；全部经由 `psig` 公共 API。
