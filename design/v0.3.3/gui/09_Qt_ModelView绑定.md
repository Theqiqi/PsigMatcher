# 09 Qt Model/View 数据绑定（v0.3.3，增量）

- 版本：v0.3.3
- 模式：修改型。本版**不涉及任何数据绑定改动**；下表逐条核对后标注「未受影响，跳过」。其余沿用 `design/v0.3.2/gui/09_Qt_ModelView绑定.md`。

## 一、既有绑定关系（引用）

| 视图 | 模型 | 基类 | 绑定方式 |
| --- | --- | --- | --- |
| `ProcessSelectionPage::table()`（`HoverRowTableView`） | `ProcessTableModel` | `QAbstractTableModel` | `QTableView::setModel`；图标经 `Qt::DecorationRole`（`setIcon` 回填） |
| `SignatureEditorPage::table()` | `SignatureTableModel` | `QAbstractTableModel`（可编辑） | `setModel` + `dataChanged` → ViewModel 置脏（v0.3.2 C-01） |
| `ResultPage::table()` | `ResultTableModel` | `QAbstractTableModel`（只读 + 末列可编辑） | `setModel` |
| 过滤/排序 | `QSortFilterProxyModel` | — | 结果页/特征码页链式代理 |

## 二、本版核对结论

| 检查项 | 本版结论 | 证据 |
| --- | --- | --- |
| 是否新增/删除模型 | **未受影响，跳过** | 本版 diff 未触及 `gui/src/model/` |
| 是否改列定义/`data()`/`setData()` | **未受影响，跳过** | 同上 |
| 是否改代理链/排序过滤 | **未受影响，跳过** | 同上 |
| 是否改 `DecorationRole` 语义 | **未受影响，跳过** | 进程图标回填（`ProcessTableModel::setIcon`）与窗口图标是**两个不同机制**：前者是模型列装饰（业务数据），后者是 `QApplication` 级外壳图标（本版改动） |
| 是否新增绑定到 View 的模型 | **未受影响，跳过** | 无新页面/新表 |

## 三、本版改动的「非 Model/View」部分（避免混淆）

| 改动 | 机制 | 与 Model/View 的关系 |
| --- | --- | --- |
| 多尺寸窗口图标 | `QApplication::setWindowIcon(QIcon)`（`main.cpp` 启动期一次） | **无关**：不经模型/视图，不参与 `data()` 取值 |
| exe 图标资源 | PE 资源节（`rc.exe` 编译 `app.rc`） | **无关**：构建期嵌入，运行期由 Windows 外壳读取 |
| 标题版本号 | `MainWindow::retranslateUi()` 字符串 | **无关** |

## 四、约束满足说明

- 修改模式：本版无受影响窗口的 Model/View 绑定关系，逐条标注「未受影响，跳过」，未默认略过。
- 明确区分「进程表装饰图标（模型列，业务数据，未变）」与「程序外壳图标（应用级，本版新增）」，避免把两者混为同一改动。
