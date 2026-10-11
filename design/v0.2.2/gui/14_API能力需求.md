# 14 API 能力需求分析（v0.2.2，增量）

- 模式：修改型。聚焦 v0.2.2 改动点新增/变更的静态库能力需求。
- 既有静态库：`psig`（v0.2.1），公开 API 见 `psig/include/psig.h`。

## 能力需求清单

### R-01 结果导出支持十进制地址列（C-01）
- 期望公共 API：`psig::algorithm::ResultColumnDefs()` 增加键 `address_dec`；`psig::ResultRowValues(const MatchResult&, const std::string& key)` 对 `address_dec` 返回 `std::to_string(r.address)`（十进制）。
- 输入/输出：键字符串 → 十进制地址字符串。
- 异常要求：键不存在行为不变。
- 验收：调用方在 `ResultExportOptions.columns` 含 `address_dec` 时，导出该列为十进制。

### R-02 结果导出表头可由调用方指定语言（C-05）
- 期望公共 API：`psig::ResultExportOptions` 新增字段 `std::vector<std::string> headers;`（与 `columns` 一一对应，可空）。
- `ExportResultsCsv/Db/Txt`：若 `headers` 非空且与 `columns` 等长，则使用 `headers[i]` 作为第 i 列表头；否则回落 `ResultColumnDefs()` 默认中文。
- 输入/输出：调用方传翻译后表头 → 文件表头为对应语言。
- 验收：英文 UI 传英文表头 → 导出文件表头英文；不传 → 中文（向后兼容）。

### R-03 结果默认表头「自定义」改名「备注」（C-01）
- `ResultColumnDefs()` 中 `{"custom","自定义"}` 改为 `{"custom","备注"}`。键 `custom` 不变（导入映射不受影响）。

### R-04 特征码导入支持英文标题回映射（C-06）
- `psig::BuiltinKeyFromTitle(const std::string& title)`：在既有中文映射（名称/特征码/备注 → name/pattern/note）基础上，新增英文映射（Name/Signature/Remarks/Note → 对应 key），保证英文 UI 导出的特征码 CSV 可被中文 UI 正确导入（跨语言往返）。
- 验收：`BuiltinKeyFromTitle("Name")=="name"` 等。

## 能力覆盖判定
- 上述能力均为**既有 API 的增强/字段扩展**，静态库 `psig` 已存在，无需新建库，但需走「静态库跨流程需求接收」（步骤 16-21，见 gproto/）补充实现。
- GUI 不直接实现缺失功能、不调用平台 API；全部经由扩展后的 `psig` 公共 API。

## 备注
- 类型词汇（`text`/`int`/`enum`）与内置键（`name`/`pattern`/`note`）已由 `psig::BuiltinSignatureColumns()`/`ColumnDef` 暴露，GUI 直接复用，无需新增 API（C-02）。
- 进程名称兜底（C-03）、权限图标（C-09）为 GUI 侧展示逻辑，不涉及静态库 API 变更。
