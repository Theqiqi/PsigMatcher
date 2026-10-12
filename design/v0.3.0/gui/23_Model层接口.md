# 23 Model 层接口（v0.3.0，增量）

> 前置：`09_Qt_ModelView绑定.md`、`design/v0.2.4/gui/23_Model层接口.md`（上一版本）

## 结论：Model 层本版无改动

- `ProcessTableModel`、`SignatureTableModel`、`ResultTableModel` 与 `model/types.h` 本版均无改动（沿用 v0.2.4）。
- 本版改动集中在 ViewModel（扫描进度采样）与 View（瞬时标签样式表优化），不涉及数据模型。

## 证据

- `git diff --stat` 本版未触及 `gui/src/model/*`。

## 约束满足说明

- Model 纯净：仅依赖 Qt 与 `model/types.h`，不依赖 psig；本版无改动。
