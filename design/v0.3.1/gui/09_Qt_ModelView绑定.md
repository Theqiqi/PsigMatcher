# 09 Qt Model/View 绑定（v0.3.1，增量）

> 模式：修改型。仅更新受影响的绑定关系；其余沿用 `design/v0.2.4/gui/09_Qt_ModelView绑定.md`。

## 一、绑定关系（受影响部分）

### 三张表的整行悬停高亮（C-01~C-03）

| 组件 | 角色 | 本版变化 |
| --- | --- | --- |
| `ProcessTableModel` / `SignatureTableModel` / `ResultTableModel` | Model（数据） | **不变**（不承载悬停态；悬停属视图态） |
| `HoverRowTableView` | View（视图子类） | **新增**：跟踪「悬停行」并**按行重绘** |
| `HoverRowDelegate` | View（委托） | **修改**：仅在**本单元格 `opt.rect`** 内绘制悬停底色；清除 `State_MouseOver` 以免原生单格高亮 |

**绑定/协作机制**：

1. `HoverRowTableView` 构造时：`setMouseTracking(true)` + `setItemDelegate(new HoverRowDelegate(this))`（集中设置，页面只 `new HoverRowTableView`）。
2. `HoverRowTableView::mouseMoveEvent`：取鼠标所在行 `rowAt(pos)`；若与当前悬停行不同 → 重绘**旧行与新行**的视口矩形，更新 `m_hoverRow`，再交基类。
3. `HoverRowTableView::leaveEvent`：清空 `m_hoverRow` 并重绘旧行。
4. `HoverRowDelegate::paint`：`initStyleOption(&opt, index)` → 清除 `State_MouseOver` → 若 `index.row() == 视图->hoveredRow()` 且非选中 → `opt.backgroundBrush = kHoverColor` → 调 `QStyledItemDelegate::paint`。**不跨越 `opt.rect` 绘制。**

> 关键：整行悬停的「行」信息由视图持有，委托只读；委托对**每一个单元格**只画自己的矩形，从而不覆盖相邻列。选中态由 `SelectRows` 一次性铺满整行（不再被越界绘制覆盖），单击即时生效。

### 进程页角落按钮（C-04）

| 组件 | 变化 |
| --- | --- |
| `ProcessSelectionPage` | 表格 `setCornerButtonEnabled(false)`；不新增控件/绑定 |

### 特征码库元数据（C-05）

| 组件 | 变化 |
| --- | --- |
| `psig::Table` | 新增 `std::string targetProcess`（库级元数据） |
| `MainViewModel` | 导出时置 `table.targetProcess = m_targetName`；导入时读回并与 `m_targetName` 比对 |

## 二、选择模式（不变）

- 三张表：`setSelectionBehavior(SelectRows)` + `setSelectionMode(ExtendedSelection)`（进程/结果/特征码均沿用）。
- `selectionModel()` 仍由 ViewModel 读取（`selectedRows()`）；本版不改选择语义。

## 三、不受影响（引用既有文档）

- 特征码表可编辑列（`ItemIsEditable`）、结果表只读列、进程表只读 + 图标 `DecorationRole`、排序等绑定：沿用既有文档。
