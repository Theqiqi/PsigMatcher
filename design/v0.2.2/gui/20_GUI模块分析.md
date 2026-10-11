# 20 GUI 模块分析（v0.2.2，增量）

> 前置：`04_GUI需求分析.md`、`14_API能力需求.md`、`design/v0.2.1/gui/20_GUI模块分析.md`（上一版本）
> 说明：标注本次改动涉及的层与文件；未涉及模块引用 v0.2.1 文档。

## 1. 四层结构（不变）

```
View（只画 UI，可单独编译）  →  Model（QAbstractTableModel，纯数据）  →  ViewModel（QObject，持 View*，集中 connect，调 psig）  →  静态库 psig
```

## 2. 改动涉及的层与文件（C-01 … C-09）

| 层 | 文件 | 变更点 | 对应 |
| --- | --- | --- | --- |
| Model | `model/processtablemodel.h/.cpp` | `data()` 名称列为空时按 PID 兜底系统进程名（PID 0→空闲进程，PID 4→系统）；`DecorationRole`（名称列图标）仅在 `accessible==true` 时返回图标（C-09）；其余列照旧 | C-03、C-09 |
| Model | `model/resulttablemodel.h/.cpp` | 末列（自定义）表头 `tr("自定义")` → `tr("备注")`；导出键 `custom` | C-01 |
| Model | `model/signaturetablemodel.h/.cpp` | `headerData` 内置列（`name`/`pattern`/`note`）走 `tr()` 随语言；自定义列沿用 `ColumnDef.title`（不随语言）；`retranslate()` 重发 `headerDataChanged` | C-06 |
| Model | `model/types.h` | `ProcessRow` 沿用（`icon` + `accessible`） | — |
| View | `view/columnmanagerdialog.h/.cpp` | 键/类型由 `QLineEdit` 改为 `QComboBox`（下拉）：键选项来自 `psig::BuiltinSignatureColumns()` 的键；类型选项固定（文本/数字等）；暴露 `keyCombo()`/`typeCombo()` getter | C-02 |
| View | `view/processselectionpage.h/.cpp` | 初始列宽：Name = 默认×2，Accessible = PID 宽，Path 自适应（stretch）；仍可由用户拖拽调整 | C-07 |
| View | `view/mainwindow.h/.cpp` | 视图菜单新增 `m_toggleLogAction`（显隐日志 dock）；**移除**视图菜单中的「清空日志」动作；`m_logEdit` 开启 `setContextMenuPolicy(CustomContextMenu)` 并暴露 `logContextMenu()`（含清空日志动作） | C-08 |
| ViewModel | `viewmodel/mainviewmodel.h/.cpp` | `resultColumnDefs()` 增加 `address_dec`（地址(十进制)）选项、`custom` 标题→备注；`exportOptions()` 按当前语言构造 `headers` 传入；`onReselectProcess` 弹窗询问保存特征码表；`startIconLoad`/`applyProcessIcon` 仅当 `accessible==true` 注入图标；`onManageColumns`/`applyInitialColumns` 由下拉源取键；`tableFromModel`/`toPsigColumn` 导出时内置列标题按键翻译、自定义列用原标题 | C-01、C-02、C-04、C-05、C-06、C-09 |
| 静态库 | `psig` | 已由静态库流程扩展（`address_dec` 键、`ResultExportOptions.headers`、`custom` 标题→备注、`BuiltinKeyFromTitle` 英文回映射） | C-01、C-05、C-06 |

## 3. 未涉及模块（引用 v0.2.1）

- `model/signaturetablemodel.*` 的 `addRow/addRows`（沿用 v0.2.1）。
- `view/signatureeditorpage.*`、`view/resultpage.*`、`view/aboutdialog.*`、`view/exportcolumnsdialog.*`（沿用 v0.2.1）。
- 语言菜单、i18n 基础机制、`retranslateUi`/`retranslate` 框架（沿用 v0.2.1，本次仅扩充翻译条目）。
- `src/main.cpp`、`gui/CMakeLists.txt`、`src/CMakeLists.txt`（本次仅追加翻译源条目，无结构性变更）。
- 资源 `res/style.qss`、`res/i18n/*.ts`/`.qm`、`res/resources.qrc`（沿用，仅新增/修改翻译条目）。

## 约束满足说明

- GUI 分为 View/Model/ViewModel/静态库 4 层；View 只画 UI 可单独编译。
- 已标注改动涉及的层与文件；未涉及模块引用 v0.2.1 文档。
- 静态库扩展经 gproto 跨流程完成，GUI 仅消费其公共 API。
