# 16 API 充分性审查（v0.2.0）

> 前置：`14_API能力需求.md`、`psig/include/psig.h`
> 目标：审查 psig 公开 API 是否满足全部改动需求并支持并发调用。

## 1. 逐能力审查

| 能力 | 现有公共 API | 满足？ | 结论 |
| --- | --- | --- | --- |
| CAP-1 CSV 携带模板 | `Table{columns,rows}` + `Import/ExportSignaturesCsv` | **否** | `Table` 无模板字段，CSV 序列化无模板 |
| CAP-2 DB 携带模板 | `Import/ExportSignaturesDb` | **否** | DB 仅有 `sig_columns`/`sig_rows`，无元数据表 |
| CAP-3 结果导出携带模板 | `ExportResultsCsv/Db/Txt` | **否** | 无模板参数 |
| CAP-4 结果自定义列 | `MatchResult` | **否** | `MatchResult` 无 `custom` 字段 |
| CAP-5 结果选列导出 | `ExportResultsCsv/Db/Txt` | **否** | 无列选择参数 |
| CAP-6 结果集删行 | — | 不需要 | GUI/Model 层 |
| CAP-7 纯 GUI 交互 | — | 不需要 | GUI 层 |
| （既有）枚举/解析/扫描/基础导入导出 | `EnumerateProcesses`/`ParsePattern`/`ScanProcesses`/`Import*/Export*` | 是 | 直接调用，行为不变 |

## 2. 并发审查

- 现有 API 均为无状态/线程安全（`ScanProcesses` 支持取消与进度回调）；本次新增能力为纯数据 I/O，天然线程安全。
- 结论：并发调用无阻碍。

## 3. 审查结论

- **不满足**：CAP-1..CAP-5 无对应公共 API。
- 处置：进入 **缺失能力分析（步骤 19）**，生成缺失需求并触发**静态库流程调用（步骤 20）**。
- 约束确认：
  - ViewModel 只直接调静态库公共 API；匹配对象只能是 psig 公共 API，禁止用 OS API 充当能力提供方。
  - 禁止在 GUI 中自行实现缺失能力或编造数据；在静态库完成契约返回前，GUI 停止 ViewModel 层相关编码。

## 4. 约束满足说明

- 已审查全部改动能力；不足 → 跳转静态库流程补充 API（见 17/18 文档）。
