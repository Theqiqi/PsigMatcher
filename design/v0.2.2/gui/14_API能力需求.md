# 14 API 能力需求分析（v0.2.1，增量）

> 前置：`11_GUI执行路径.md`、`psig` 公共头文件 `psig/include/psig.h`、`design/v0.2.0/gproto/77_静态库完成契约.md`
> 模式：修改/添加功能 → 聚焦改动点**新增**的能力需求；已有 API 可覆盖的直接调用。

## 1. 改动点 → 能力需求

| 能力编号 | 关联改动点 | 能力需求 | 期望公共 API | 输入 | 输出 | 异常要求 | 验收标准 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| CAP-1 | C-03 | 特征码表 CSV 导出**写 UTF-8 BOM**；导入**兼容带/不带 BOM** | `ExportSignaturesCsv` / `ImportSignaturesCsv` | 文件路径 + `Table` | 成功/失败；文件首部含 BOM | 无 BOM 旧文件仍可导入；不可读写置 err | 导出文件首 3 字节 = `EF BB BF`；往返一致 |
| CAP-2 | C-03 | 结果 CSV 导出**写 UTF-8 BOM** | `ExportResultsCsv` | 文件路径 + 结果 + 选项 | 成功/失败；文件首部含 BOM | 同上 | Excel 打开中文表头不乱码；往返一致 |
| CAP-3 | C-12 | **提取指定进程图标**（ARGB 位图） | 新 API `ProcessIcon(pid, …)` | pid | 宽/高 + ARGB 像素字节（可空） | 进程无图标/取失败 → 返回空（GUI 用系统默认）；不抛异常 | 有图标的进程返回非空位图且尺寸>0 |
| CAP-4 | C-12 | **系统默认进程图标** | 新 API `DefaultProcessIcon(…)` 或 `ProcessIcon` 对空值返回默认 | 无 | 宽/高 + ARGB 字节 | 始终可返回（内置/系统默认） | 无图标进程显示默认图标而非空白 |
| CAP-5 | C-01/C-04/C-05 | 对话框按钮生效 | 无需静态库 | — | — | — | GUI 层实现 |
| CAP-6 | C-02 | 界面语言切换 | 无需静态库 | — | — | — | GUI 层实现 |
| CAP-7 | C-06/C-08/C-09/C-10/C-11/C-07/C-13/C-14 | 纯 GUI 交互/构建/表现 | 无需静态库 | — | — | — | GUI 层实现 |

## 2. 现有 API 覆盖情况

| 能力 | 现有 API | 是否覆盖 |
| --- | --- | --- |
| CAP-1/CAP-2 | `Import/ExportSignaturesCsv`、`ExportResultsCsv`（`SerializeCsv` 无 BOM） | **否**（需扩展：写 BOM + 导入剥离） |
| CAP-3/CAP-4 | `ProcessInfo`（仅 pid/name/path/accessible，无图标） | **否**（需新增图标 API） |
| CAP-5/CAP-6/CAP-7 | — | 不需要静态库 |

## 3. 约束满足说明

- 逐条列出改动点所需能力、期望公共 API、输入输出、异常要求、验收标准。
- 已有 API 能覆盖的（枚举/解析/扫描/基础导入导出）→ 直接调用；不足的（CAP-1..CAP-4）→ 记入缺失能力，走静态库流程（步骤 17-20）。
- 匹配对象仅为静态库公共 API；**GUI 不得用 OS API 充当能力提供方**（图标必须由 psig 提供），不得自行实现缺失能力或编造数据。
