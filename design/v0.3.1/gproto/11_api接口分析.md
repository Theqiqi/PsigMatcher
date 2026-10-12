# 11 api 接口分析（psig v0.3.1）

> 前置：`10_职责和边界变更.md`

## 公共 API 变更

| 符号 | 变更 | 兼容性 |
| --- | --- | --- |
| `struct Table` | 增字段 `std::string targetProcess;` | 向后兼容（新增字段；既有函数签名不变） |
| `ExportSignaturesDb(path, table, err)` | 行为增强（写 `target_process`） | 兼容 |
| `ImportSignaturesDb(path, err)` | 行为增强（读 `target_process`） | 兼容 |

## 不变

- `ExportSignaturesCsv` / `ImportSignaturesCsv`（CSV 路径不携带该元数据）。
- 列/行结构、`ColumnDef`、`TableRow`、其余 API。

## 接口契约

- 不抛异常；错误经 `err` 出参。
- `targetProcess` 为空 → 不写键；导入缺键 → 空串。
