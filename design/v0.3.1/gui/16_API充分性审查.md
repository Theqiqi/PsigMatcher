# 16 API 充分性审查（v0.3.1）

> 前置：`14_API能力需求.md`、`psig/include/psig.h`。

## 审查对象：本版改动点所需静态库能力

| 改动点 | 所需能力 | 现有公共 API | 是否满足 |
| --- | --- | --- | --- |
| C-01~C-04 | 无（纯 Qt 视图层） | — | 不适用 |
| C-05 | 特征码库携带库级目标进程名 | `Table{columns,rows,templateText}`、`ExportSignaturesDb`、`ImportSignaturesDb` | **不满足**（`Table` 无库级进程字段；DB 读写无该元数据） |

## 结论：部分不满足 → 需补充静态库能力

- `ExportSignaturesDb` / `ImportSignaturesDb` 签名可保持不变（仍收/返 `Table`），但 `psig::Table` 需新增库级字段 `std::string targetProcess`，并由 `exchange_signatures.cpp` 通过 `sig_meta` 持久化。
- 属**公共头变更 + api 层实现变更** → 生成缺失需求，跳转静态库流程补充。

## 并发与隔离约束

- C-05 的交换 API 为同步文件 I/O，不涉及并发调用（沿用既有单线程调用方式）。
- ViewModel 直接调静态库 API（`psig::ImportSignaturesDb` / `ExportSignaturesDb`）；匹配对象仅为 `psig` 公共 API。
- **禁止**用 OS API 充当能力提供方；**禁止**在 GUI 自行实现 DB 读写或编造元数据。

## 约束满足说明

- 满足 → 继续 GUI 设计；不足 → 跳转静态库流程补充 API（见 `17_API缺失需求.md`、`18_静态库流程交接记录.md`）。
