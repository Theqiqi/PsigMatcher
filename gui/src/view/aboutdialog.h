// view/aboutdialog.h - 关于对话框（只画 UI）
#pragma once

#include <QDialog>

class QLabel;
class QDialogButtonBox;

class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);

    void setInfo(const QString& text);
    QString info() const;
    QDialogButtonBox* buttonBox() const { return m_buttons; }

private:
    QLabel* m_label = nullptr;
    QDialogButtonBox* m_buttons = nullptr;
};
