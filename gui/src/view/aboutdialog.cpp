// view/aboutdialog.cpp - 关于对话框实现（仅 UI）
#include "view/aboutdialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("关于"));
    setMinimumWidth(360);

    m_label = new QLabel(this);
    m_label->setTextFormat(Qt::PlainText);
    m_label->setWordWrap(true);
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_label);
    layout->addWidget(m_buttons);
}

void AboutDialog::setInfo(const QString& text) { m_label->setText(text); }

QString AboutDialog::info() const { return m_label->text(); }
