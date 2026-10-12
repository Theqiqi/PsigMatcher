# v0.3.2 gproto CHANGELOG（静态库侧）

> 版本：v0.3.2
> 输入：`.idea/v0.3.2.md`（GUI 侧修改型流程）

## 结论：本版**不改静态库 `psig`**

- 三个改动点（未保存检测、导入与保存解耦、关闭提示、程序改名）全部落在 GUI 侧：
  - 未保存状态由 ViewModel 的 `m_sigDirty` 维护，数据来源为 GUI Model 信号（Qt 侧），静态库不参与。
  - 导入覆盖确认、关闭提示均为 GUI 交互，不新增 `psig` 能力。
  - 程序改名是构建/入口/脚本层面，与静态库 API 无关。
- 静态库公共 API 覆盖情况见 `../gui/14_API能力需求.md` ~ `../gui/19_API能力匹配分析.md`：
  本版所需全部能力（`ImportSignaturesDb/Csv`、`ExportSignaturesDb/Csv`、`Table`/`ColumnDef`）在 v0.3.1 已存在，**无缺失能力**。

## 因此本目录不产生静态库设计文档

- 不进入 `.flow/带GUI的单体程序的业务系统静态库.md` 流程；
- `psig` 公共头 `psig/include/psig.h` 的结构与 API 不变；
- 版本字符串随 GUI 版本统一更新为 `v0.3.2`（仅注释/常量，无行为变化），见 `../gui/25_静态库API调用映射.md`。
