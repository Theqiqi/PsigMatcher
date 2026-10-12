# 05 GUI 功能列表与窗口定义（v0.3.3，增量）

- 版本：v0.3.3
- 模式：修改型。仅列本版**新增/修改**的功能与窗口定义；其余沿用 `design/v0.3.2/gui/05_GUI功能列表与窗口定义.md`。

## 一、窗口定义（增量）

| 窗口 | 类型 | 本版变化 |
| --- | --- | --- |
| `MainWindow` | `QMainWindow`（主窗口，1100×720） | **图标**：单尺寸 → 多尺寸 `QIcon`（7 尺寸）；**标题**版本号 → v0.3.3；布局/页面/菜单/快捷键**不变** |
| `SignatureEditorPage` / `ProcessSelectionPage` / `ResultPage` | `QWidget`（`QStackedWidget` 页面） | **不变** |
| `AboutDialog` / `ColumnManagerDialog` / `ExportColumnsDialog` | 模态 `QDialog` | **不变** |
| 未保存提示（`QMessageBox`，v0.3.2 新增） | 模态 | **不变** |

> 本版**无新增窗口、无删除窗口**。

## 二、功能列表（增量）

| 功能编号 | 功能 | 变更类型 | 定义 | 入口 |
| --- | --- | --- | --- | --- |
| F-ICON-01 | exe 内嵌图标资源 | **新增** | 生产 exe 携带多尺寸图标资源（PE `RT_GROUP_ICON`/`RT_ICON`），供 Windows 外壳显示 | 无（构建期生效） |
| F-ICON-02 | 多尺寸窗口图标 | **修改** | 启动时用 7 个尺寸装配 `QIcon` 并设为应用窗口图标，覆盖标题栏/任务栏/Alt-Tab | 无（启动期生效） |
| F-ICON-03 | 图标资源生成 | **新增** | 由脚本从几何规则生成 `app.ico` 与 `app_<N>.png`，可随时重建 | 命令行 `python3 gui/script/gen_app_icon.py` |
| F-DOC-01 | 英文 README（默认） | **新增** | 仓库首页文档，英文，含语言切换行 | GitHub 首页 |
| F-DOC-02 | 简体中文 README | **新增** | 中文对等文档，含语言切换行 | 英文版首行链接 |
| F-TITLE-01 | 标题版本号 | **修改** | 标题随版本更新为 v0.3.3 | 无（启动期生效） |

## 三、既有功能（未受影响）

- 进程枚举/刷新/图标回填、目标进程选择、特征码表编辑（增/删/复制/粘贴/列管理/导入/导出）、扫描（进度/取消）、结果表（排序/筛选/列宽/导出）、批量操作、语言切换、关于框、未保存检测与关闭提示：**未受影响，引用 `design/v0.3.2/gui/05_GUI功能列表与窗口定义.md`**。
- 本版**不新增/修改任何用户可见的交互入口**（快捷键表与 v0.3.2 逐条相同）。

## 四、双语 README 结构对照（R-07）

两版**章节一一对应**（英文版为默认渲染）；实测两文件均为 **23 个标题行**（其中 7 个 `#` 行是代码块内的注释/命令），正文一级/二级章节 **12 节逐一对应**：

| # | `README.md`（English） | `README.zh-CN.md`（简体中文） |
| --- | --- | --- |
| 0 | `**English** | [简体中文](README.zh-CN.md)` | `[English](README.md) | **简体中文**` |
| 1 | Title + one-line description | 标题 + 一句话简介 |
| 2 | Highlights | 亮点 |
| 3 | Repository layout | 仓库结构 |
| 4 | Requirements | 环境要求 |
| 5 | Build & run | 构建与运行 |
| 6 | Tests | 测试 |
| 7 | Application icon | 应用程序图标 |
| 8 | Static library API | 静态库公共 API |
| 9 | Design and process documents | 设计与流程文档 |
| 10 | Project conventions | 项目约定 |
| 11 | Version notes | 各版本要点 |
| 12 | FAQ | 常见问题 |
| 13 | License | 许可证 |

## 五、约束满足说明

- 窗口定义使用 Qt 控件术语（`QMainWindow`/`QWidget`/`QStackedWidget`/`QDialog`/`QMessageBox`）。
- 修改模式：仅更新受影响窗口（`MainWindow` 的图标与标题）的功能定义；未受影响功能引用既有文档。
