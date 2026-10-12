# 25 静态库 API 调用映射（v0.3.1，增量）

> 前置：`24_ViewModel接口.md`、`psig/include/psig.h`（v0.3.1）。

## VM 业务方法 → psig 公共 API

| VM 方法 | psig 公共 API | 说明 |
| --- | --- | --- |
| `MainViewModel::exportSignaturesToPath` | `bool psig::ExportSignaturesDb(const std::string&, const Table&, std::string&)` | 入参 `Table` 含 `targetProcess=m_targetName` |
| `MainViewModel::importSignaturesFromPath` | `psig::Table psig::ImportSignaturesDb(const std::string&, std::string&)` | 返回 `Table` 含库级 `targetProcess` |
| （CSV 分支） | `psig::ExportSignaturesCsv` / `ImportSignaturesCsv` | 不变（不携带 `targetProcess`） |
| `tableFromModel` | `psig::Table` 结构 | 新增字段 `targetProcess`（库级元数据） |
| `applyTableToModel` | `psig::Table` 结构 | 不读取 `targetProcess`（库级元数据不落入特征码表） |

## 数据结构映射

| psig 结构/字段 | GUI 侧 |
| --- | --- |
| `Table::targetProcess` | VM 的 `m_targetName`（导出）/ 导入比对基准 |
| `Table::columns` / `Table::rows` | 沿用（→ `SignatureTableModel`） |

## API 真实性

- `Table::targetProcess`：`psig.h:80` 逐字命中。
- `ExportSignaturesDb` / `ImportSignaturesDb`：`psig.h:156` / `psig.h:151` 逐字命中（签名未变）。

## 约束满足说明

- 每个 VM 业务方法对应真实静态库 API 调用；无对应 API → 已走静态库流程补齐（R-01）。
- 匹配对象仅为 psig 公共 API；不用 OS API；GUI 不自行实现 `.db` 读写。
