# 20 GUI 模块分析（v0.2.1，增量）

> 前置：`04_GUI需求分析.md`、`14_API能力需求.md`、`design/v0.2.0/gui/20_GUI模块分析.md`（基线）
> 说明：标注改动涉及的层与文件；未涉及模块引用既有文档。

## 1. 四层结构（不变）

```
View（只画 UI，可单独编译）  →  Model（QAbstractTableModel，纯数据）  →  ViewModel（QObject，持 View*，集中 connect，调 psig）  →  静态库 psig
```

## 2. 改动涉及的层与文件

| 层 | 文件 | 变更 |
| --- | --- | --- |
| View | `view/mainwindow.h/.cpp` | 新增「语言」菜单；`retranslateUi()`；日志着色（`appendLog` 富文本）；Release 去控制台不影响 View |
| View | `view/processselectionpage.h/.cpp` | 筛选框移到底部；表格开启排序（`setSortingEnabled`）；名称列图标由 Model 提供 |
| View | `view/signatureeditorpage.h/.cpp` | 右键菜单新增「新增多行…」动作 getter |
| View | `view/resultpage.h/.cpp` | 配色随全局 qss（白底） |
| View | `view/columnmanagerdialog.h/.cpp` | 暴露内部按钮 getter（已有）；由 VM 连接 |
| View | `view/aboutdialog.h/.cpp` | 暴露 `buttonBox()`（已有）；由 VM 连接 |
| View | `view/exportcolumnsdialog.h/.cpp` | 暴露 `buttonBox()`（已有）；由 VM 连接 |
| Model | `model/types.h` | `ProcessRow` + `QIcon icon` |
| Model | `model/processtablemodel.h/.cpp` | `DecorationRole`（名称列图标）；`sort(column, order)` |
| Model | `model/signaturetablemodel.h/.cpp` | `addRow(默认值)`；`addRows(count, 默认值)` |
| ViewModel | `viewmodel/mainviewmodel.h/.cpp` | 对话框按钮接线；i18n（语言切换）；新增一行/多行；图标注入；启动自动刷新；刷新后重应用筛选 |
| 资源 | `res/style.qss` | 白底统一 + 日志/状态着色 |
| 资源 | `res/i18n/psig_gui_zh_CN.ts`、`psig_gui_en_US.ts` + 编译 `.qm` | **新增**：翻译源与产物 |
| 资源 | `res/resources.qrc` | 加入 `.qm` |
| 入口 | `src/main.cpp` | 安装 `QTranslator`（按系统语言）；注册语言切换入口 |
| 构建 | `gui/CMakeLists.txt`、`src/CMakeLists.txt` | Release `WIN32_EXECUTABLE`；`Qt6::LinguistTools` 编译 `.ts`→`.qm` |
| 静态库 | `psig` | 已由静态库流程扩展（v0.2.1：CSV BOM、进程图标） |

## 3. 未涉及模块（引用 v0.2.0）

- `model/resulttablemodel.*`（沿用 v0.2.0 的 10 列/末列可编辑/删行）。

## 约束满足说明

- GUI 分为 View/Model/ViewModel/静态库 4 层；View 只画 UI 可单独编译。
- 已标注改动涉及的层与文件；未涉及模块引用既有文档。
