# 23 Model 层接口（v0.3.3，增量）

- 版本：v0.3.3
- 前置：`09_Qt_ModelView绑定.md`、`21_职责和边界.md`

## 一、本版 Model 层接口变化：**无（未受影响）**

| Model 类 | 类型 | 本版变化 |
| --- | --- | --- |
| `SignatureTableModel` | `QAbstractTableModel`（可编辑） | **未受影响**（无 diff） |
| `ProcessTableModel` | `QAbstractTableModel`（含 `setIcon` 装饰图标回填） | **未受影响**（无 diff） |
| `ResultTableModel` | `QAbstractTableModel`（只读 + 末列可编辑） | **未受影响**（无 diff） |

- 数据结构（`gui/src/model/types.h` 的 `ProcessRow`/`SignatureRow`/`ResultRow` 等）与列定义：**未受影响**。
- 证据：本版 diff 未触及 `gui/src/model/`（见 `20_GUI模块分析.md` 第二节精确清单）。

## 二、易混淆点澄清：模型装饰图标 ≠ 程序图标

| 项 | `ProcessTableModel::setIcon(row, QIcon)`（既有，未变） | `QApplication::setWindowIcon(QIcon)`（本版新增） |
| --- | --- | --- |
| 所属层 | **Model**（`Qt::DecorationRole` 的数据提供方） | **入口**（应用级配置） |
| 数据来源 | 静态库 `ProcessIcon`/`DefaultProcessIcon` → ViewModel 转 `QIcon` 后回填 | `.qrc` 中 `:/app_<N>.png` |
| 作用范围 | 进程表某一行的单元格装饰 | 主窗口标题栏/任务栏/Alt-Tab |
| 本版是否改动 | ❌ 否 | ✅ 是（`main.cpp`） |

> 两者互不影响：本版**不改变**任何 `data()`/`setData()`/`flags()` 语义。

## 三、Model/View 契约（本版复核）

| 契约项 | 复核结论 |
| --- | --- |
| 只读表的 `flags()` 不含 `ItemIsEditable` | ✅ 未变（`ResultTableModel`/`ProcessTableModel` 未纳入 diff） |
| `setData` 后 `emit dataChanged` 的范围正确 | ✅ 未变 |
| Model 不 include `view/`、`viewmodel/`、静态库头 | ✅ 未变（无 diff） |

## 四、约束满足说明

- 本版 Model 层接口零变化，逐项标注「未受影响」并给出证据；未涉及内容引用 `design/v0.3.2/gui/23_Model层接口.md`。
