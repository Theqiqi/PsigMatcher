// main.cpp - 组装各层（最后实现）
#include <QApplication>
#include <QFile>
#include <QIcon>

#include "view/mainwindow.h"
#include "viewmodel/mainviewmodel.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("psig_gui"));
    QApplication::setOrganizationName(QStringLiteral("psig"));

    QFile qss(QStringLiteral(":/style.qss"));
    if (qss.open(QIODevice::ReadOnly)) app.setStyleSheet(QString::fromUtf8(qss.readAll()));
    app.setWindowIcon(QIcon(QStringLiteral(":/app.png")));

    MainWindow view;          // 1) 创建 View
    MainViewModel viewModel;  // 2) 创建 ViewModel
    viewModel.install(&view); // 3) 注入 View 指针并集中 connect
    view.show();              // 4) 显示
    return app.exec();        // 5) 进入事件循环
}
