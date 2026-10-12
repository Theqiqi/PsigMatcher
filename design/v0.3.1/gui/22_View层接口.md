# 22 View 层接口（v0.3.1，增量）

> 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`
> 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect**；暴露控件 getter、公开 UI 方法。

## HoverRowTableView（新增，`view/hoverrowtableview.h`）

```cpp
class HoverRowTableView : public QTableView {
    Q_OBJECT
public:
    explicit HoverRowTableView(QWidget* parent = nullptr);
    int hoveredRow() const { return m_hoverRow; }   // -1 表示无悬停行
protected:
    void mouseMoveEvent(QMouseEvent* event) override;   // 更新悬停行并按行重绘
    void leaveEvent(QEvent* event) override;            // 清空悬停行并重绘
private:
    void updateRow(int row);                            // 重绘该行的视口矩形
    int m_hoverRow = -1;
};
```

- 构造：`setMouseTracking(true)` + `viewport()->setMouseTracking(true)` + `setItemDelegate(new HoverRowDelegate(this))`（集中装配，页面只 `new HoverRowTableView`）。
- `mouseMoveEvent`：`row = indexAt(event->pos()).row()`；若 `row != m_hoverRow` → 重绘旧行与新行、更新 `m_hoverRow`；随后交基类。
- `leaveEvent`：重绘旧行、`m_hoverRow = -1`；交基类。
- `updateRow(row)`：`row<0 || 无 model || 越界` → 提前 return；否则 `viewport()->update(0, rowTop, viewport()->width(), rowHeight)`。
- 只画 UI；无 signals/slots/connect。

## HoverRowDelegate（修改，`view/hoverrowdelegate.h`）

```cpp
class HoverRowDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HoverRowDelegate(QObject* parent = nullptr);
protected:
    // 关键：在 initStyleOption 中注入悬停底色（基类 paint 内部会再次调用本虚函数，故对两处生效）
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};
```

- `initStyleOption`：调用基类后，
  - 清除 `option->state & QStyle::State_MouseOver`（避免原生**单格** hover 高亮）；
  - 若 `option->widget` 为 `HoverRowTableView` 且 `index.row() == hoveredRow()` 且非选中 → `option->backgroundBrush = kHoverColor`。
- **不再**在 `paint` 中跨越 `opt.rect` 绘制；不再提供 `attach()`。
- 只画 UI；不发射信号、不写 Model。

## 页面改动

| 页面 | 改动 |
| --- | --- |
| `ProcessSelectionPage` | `m_table = new HoverRowTableView(this);` + `m_table->setCornerButtonEnabled(false);`（getter 类型仍为 `QTableView*`） |
| `SignatureEditorPage` | `m_table = new HoverRowTableView(this);` |
| `ResultPage` | `m_table = new HoverRowTableView(this);` |

> 三页 getter 保持返回 `QTableView*`（基类指针），不改变既有 VM 使用方式。

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层；`HoverRowTableView` 头文件只 `#include <QTableView>`。
- 无 signals/private slots/connect；控件经 getter 暴露。
