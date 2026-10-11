# 17 原子化 api 分析（psig v0.2.0）

> 前置：`12_接口细化设计.md`、`design/v0.1.0/gproto/17_原子化api分析.md`（基线）

## 分析结论

本次改动**无需新增原子化 API**，复用既有：

| 原子化能力 | 复用点 |
| --- | --- |
| `SqliteOpen/Close/Exec/Run/Query/TableExists` | 建/读写 `sig_meta`、`result_meta` |
| `ReadTextFile/WriteTextFile` | CSV/TXT 读写 |

## 原子化边界确认

- 原子化层不含「模板」业务语义：仅提供通用 SQL 执行与文件读写；「模板」语义由接口层组装、字符串细节由算法层处理。

## 约束满足说明

- 复用既有原子化 API；不引入平台 API 作为能力提供方。
