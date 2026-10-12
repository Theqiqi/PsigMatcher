# 22 View 层接口（v0.2.4，增量）

> 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`
> 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect**；暴露控件 getter、公开 UI 方法。

## HoverRowDelegate（新增）

```cpp
// view/hoverrowdelegate.h
class HoverRowDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HoverRowDelegate(QObject* parent = nullptr);
    static void attach(QTableView* view);   // setMouseTracking(true) + setItemDelegate(new HoverRowDelegate(view))
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
```

- `paint`：若 `option.state & QStyle::State_MouseOver` → 以悬停色（选中色 `#3a6ea5` 同色系更淡）填充**整行**背景（`option.rect` 横向扩展至视口宽），再绘制单元格内容；否则调用基类 `paint`。
- `attach(view)`：在表格上启用整行悬停（`setMouseTracking(true)` + `setItemDelegate`）。
- 仅绘制，不发射信号、不改 Model。

## ProcessSelectionPage / SignatureEditorPage / ResultPage（增量）

- 三张表构造时调用 `HoverRowDelegate::attach(m_table)`（C-02）。
- `selectionBehavior` 保持 `SelectRows`（选中整行语义不变）。

## SignatureEditorPage（增量）

- 导入/导出特征码文件对话框过滤器改为 `*.db`（移除 `*.csv` 过滤器项）（C-05）。
- 不新增控件；模板文本输入框不在本页（沿用）。

## ResultPage（增量）

- 导出结果文件对话框过滤器改为 `*.csv` / `*.txt`（移除 `*.db` 过滤器项/菜单项）（C-05）。
- 保留模板文本输入框（模板文本仅属结果页，C-04）。

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层；无 signals/slots/connect；控件经 getter 暴露。
- `HoverRowDelegate` 属 View 层，仅负责绘制；不引入业务/静态库依赖。
