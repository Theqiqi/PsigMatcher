# 16 API 充分性审查（v0.2.4）

> 前置：`14_API能力需求.md`、`psig/include/psig.h`
> 目标：审查 psig 公开 API 是否满足全部改动需求并支持并发调用。

## 1. 逐能力审查

| 能力 | 现有公共 API | 满足？ | 结论 |
| --- | --- | --- | --- |
| R-01 进程名来源（C-01） | `EnumerateProcesses()`（内部 `atomic::ProcessNamePath` 依赖 `OpenProcess`，受保护进程返回空名） | **否** | 需修正内部实现：名称改由 Toolhelp `szExeFile` 提供 |
| R-02 结果摘要随语言（C-03） | `ResultExportOptions{templateText, columns, headers}` | **否** | 无摘要字段；`ExportResultsTxt` 摘要行写死中文 |
| C-03 结果列头「备注」随语言 | `ResultExportOptions.headers`（已支持自定义表头） | 是（psig） | 中文系 GUI 侧 `resultHeaderForKey` 硬编码 → **GUI 修复**，无需 psig |
| C-04 特征码表解耦模板 | `Table::templateText`（保留） | 是 | GUI 侧不读不写即达成，无需 psig |
| C-05 入口收敛 | `ImportSignaturesCsv`/`ExportSignaturesCsv`/`ExportResultsDb`（保留） | 是 | GUI 对话框过滤器，无需 psig |
| C-02 整行 hover | — | 不需要 | GUI 层（delegate/qss） |
| （既有）枚举/解析/扫描/图标/基础导入导出 | `EnumerateProcesses`/`ParsePattern`/`ScanProcesses`/`ProcessIcon`/`Import*`/`Export*` | 是 | 直接调用，行为不变（除 R-01 修正） |

## 2. 并发审查

- 现有 API 均为无状态/线程安全（`ScanProcesses` 支持取消与进度回调）。
- 本次新增/修正能力（进程名来源、导出摘要字段）为只读枚举与纯数据 I/O，天然线程安全。
- 结论：并发调用无阻碍。

## 3. 审查结论

- **不满足**：R-01、R-02 需静态库变更。
- 处置：进入 **缺失能力分析（步骤 19）**，生成缺失需求并触发**静态库流程调用（步骤 20）**。
- 约束确认：
  - ViewModel 只直接调静态库公共 API；匹配对象只能是 psig 公共 API，**禁止用 OS API 充当能力提供方**（进程名必须由 psig 提供）。
  - 禁止在 GUI 中自行实现缺失能力或编造数据；在静态库完成契约返回前，GUI 停止 ViewModel 层相关编码。

## 4. 约束满足说明

- 已审查全部改动能力；不足 → 跳转静态库流程补充 API（见 17/18 文档）。
