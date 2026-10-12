# 20 GUI 模块分析（v0.2.4，增量）

> 前置：`04_GUI需求分析.md`、`14_API能力需求.md`、`design/v0.2.2/gui/20_GUI模块分析.md`（上一版本）
> 说明：标注本次改动涉及的层与文件；未涉及模块引用 v0.2.2 文档。

## 1. 四层结构（不变）

```
View（只画 UI，可单独编译）  →  Model（QAbstractTableModel，纯数据）  →  ViewModel（QObject，持 View*，集中 connect，调 psig）  →  静态库 psig
```

## 2. 改动涉及的层与文件（C-01 … C-05）

| 层 | 文件 | 变更点 | 对应 |
| --- | --- | --- | --- |
| Model | `model/processtablemodel.cpp/.h` | 名称列 `data()`：`r.name` 非空时直接返回（不再出现空名）；保留极端兜底，正常不触发 | C-01 |
| View | `view/hoverrowdelegate.h/.cpp` | **新增**：自定义 `QStyledItemDelegate`，`paint` 检测 `State_MouseOver` 时绘制**整行**淡色背景 | C-02 |
| View | `view/processselectionpage.cpp/.h`、`view/signatureeditorpage.cpp/.h`、`view/resultpage.cpp/.h` | 三张 `QTableView` `setMouseTracking(true)` + `setItemDelegate(new HoverRowDelegate)`；`retranslate` 不需变化 | C-02 |
| View | `view/signatureeditorpage.cpp` | 导入/导出特征码文件对话框过滤器改为仅 `*.db` | C-05 |
| View | `view/resultpage.cpp` | 导出结果文件对话框过滤器改为仅 `*.csv`/`*.txt`（移除 `*.db` 菜单项/过滤器） | C-05 |
| View | `res/style.qss` | 新增悬停整行背景色规则（若采用 QSS 方案；与 delegate 二选一或并存） | C-02 |
| ViewModel | `viewmodel/viewmodel_helpers.cpp` | `resultHeaderForKey("custom")` 由硬编码 `"备注"` 改为 `QObject::tr("备注")`（随语言） | C-03 |
| ViewModel | `viewmodel/result.cpp` | `exportResultsToPath`/`exportOptions` 传 `summary`（本地化摘要）；`onExportResultsDb` 入口移除（仅保留 Csv/Txt） | C-03、C-05 |
| ViewModel | `viewmodel/signature.cpp` | `tableFromModel` **不再**写入 `t.templateText`；`applyTableToModel` **不再**回填结果页模板 | C-04 |
| 静态库 | `psig` | 已由静态库流程修正（`EnumerateProcesses` 名称来源、`ResultExportOptions.summary`） | C-01、C-03 |

## 3. 未涉及模块（引用 v0.2.2）

- `model/signaturetablemodel.*`、`model/resulttablemodel.*`（除表头经 `resultHeaderForKey` 外无改动）。
- `view/mainwindow.*`、`view/columnmanagerdialog.*`、`view/exportcolumnsdialog.*`、`view/aboutdialog.*`。
- `viewmodel/{bind,process,scan,language,mainviewmodel}.*`（除 `result.cpp`/`signature.cpp`/`viewmodel_helpers.cpp` 外无改动）。
- 语言菜单、i18n 基础机制、`retranslate` 框架（沿用；本次仅扩充/修正翻译条目）。
- `src/main.cpp`、`gui/CMakeLists.txt`、`src/CMakeLists.txt`（本次仅追加 delegate 源文件与翻译条目）。

## 约束满足说明

- GUI 分为 View/Model/ViewModel/静态库 4 层；View 只画 UI 可单独编译。
- 已标注改动涉及的层与文件；未涉及模块引用 v0.2.2 文档。
- 静态库修正经 gproto 跨流程完成，GUI 仅消费其公共 API。
