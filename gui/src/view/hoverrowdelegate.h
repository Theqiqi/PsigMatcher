// view/hoverrowdelegate.h - 整行悬停高亮委托（只画 UI）
#pragma once

#include <QStyledItemDelegate>

class QTableView;

// 鼠标悬停时高亮整行（悬停色为选中色同色系、更淡）。
// 仅负责绘制：不发射信号、不修改 Model、不写日志。
class HoverRowDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HoverRowDelegate(QObject* parent = nullptr);

    // 在表格上启用整行悬停：setMouseTracking(true) + setItemDelegate(new HoverRowDelegate(view))
    static void attach(QTableView* view);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
