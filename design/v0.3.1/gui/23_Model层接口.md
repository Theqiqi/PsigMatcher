# 23 Model 层接口（v0.3.1，增量）

> 模式：修改型。

## 结论：Model 层无改动

- 本版改动点均不涉及 Model 层：
  - C-01~C-04 为 View 层绘制/控件行为（悬停态属视图态，不入 Model）。
  - C-05 的库级目标进程名属 `psig::Table` 与 VM，不入 Model（`SignatureTableModel` 只管理列/行单元格）。
- `SignatureTableModel` / `ProcessTableModel` / `ResultTableModel` 接口不变，沿用 `design/v0.2.4/gui/23_Model层接口.md`。

## 约束满足说明

- Model 纯净、无业务/格式化逻辑；不 include 静态库头；不直接调用静态库 API（本版未改）。
