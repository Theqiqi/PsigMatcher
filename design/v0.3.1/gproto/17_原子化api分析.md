# 17 原子化 api 分析（psig v0.3.1）

> 前置：`12_接口细化设计.md`

## 结论：无需新增原子化 API

- 所需能力（打开库、执行 SQL、查询、判断表存在、关闭库）**均已存在**：
  - `atomic::SqliteOpen` / `SqliteExec` / `SqliteRun` / `SqliteQuery` / `SqliteTableExists` / `SqliteClose`。
- R-01 仅在 api 层组合既有原子能力，**不新增/修改 atomic 层 API**。
