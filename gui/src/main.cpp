// main.cpp - 组装各层（最后实现）
#include <QApplication>
#include <QFile>
#include <QIcon>

#include "view/mainwindow.h"
#include "viewmodel/mainviewmodel.h"

namespace {

// C-01：窗口图标尺寸集合（与 gui/res/resources.qrc、gui/script/gen_app_icon.py 一一对应）
// 逐个尺寸加入 QIcon，由 Qt 按目标 DPI/使用场景（标题栏/任务栏/Alt-Tab）选取原生帧。
constexpr int kAppIconSizes[] = {16, 24, 32, 48, 64, 128, 256};

QIcon buildAppIcon() {
    QIcon icon;
    for (int size : kAppIconSizes) {
        icon.addFile(QStringLiteral(":/app_%1.png").arg(size), QSize(size, size));
    }
    return icon;
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("PsigMatcher"));
    QApplication::setOrganizationName(QStringLiteral("psig"));

    QFile qss(QStringLiteral(":/style.qss"));
    if (qss.open(QIODevice::ReadOnly)) app.setStyleSheet(QString::fromUtf8(qss.readAll()));
    app.setWindowIcon(buildAppIcon()); // 多尺寸窗口图标（exe 文件图标见 gui/res/app.rc）

    MainWindow view;          // 1) 创建 View
    MainViewModel viewModel;  // 2) 创建 ViewModel
    viewModel.install(&view); // 3) 注入 View 指针并集中 connect
    view.show();              // 4) 显示
    return app.exec();        // 5) 进入事件循环
}
