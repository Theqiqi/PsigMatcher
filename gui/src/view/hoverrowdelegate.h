// view/hoverrowdelegate.h - 整行悬停高亮委托（只画 UI）
#pragma once

#include <QStyledItemDelegate>

class HoverRowTableView;

// 悬停整行高亮：委托仅在自身单元格矩形内绘制悬停底色（淡色）。
// 悬停行由 HoverRowTableView 提供（见 hoverrowtableview.h）。
// 仅负责绘制：不发射信号、不修改 Model、不写日志。
class HoverRowDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HoverRowDelegate(QObject* parent = nullptr);

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};
