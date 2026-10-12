# 18 原子化 api 设计（psig v0.3.0）

> 前置：`17_原子化api分析.md`

## 结论：无原子化 api 流程图变更
- 本轮不新增/修改原子化 api，故无新的原子化流程图。
- 既有原语流程图（`OpenForRead`/`EnumRegions`/`ReadMemory`/`CloseProcessHandle`）与 v0.2.4 基线一致。

## 数据结构分析
- 无新增原子层数据结构；`data::Region` / `data::ModuleInfo` 沿用。

## 异常设计
- 全部以返回值表达（`nullptr` / 空表 / 空 buffer），不抛异常；句柄成对释放（`CloseProcessHandle`）。
- 并发场景下各线程只读句柄、只写局部数据；无跨线程异常。
