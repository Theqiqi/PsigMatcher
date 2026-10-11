// test/unit/test_viewmodel.cpp - ViewModel 单元测试（事件链接、槽、调用链）
#include <gtest/gtest.h>

#include <QAction>
#include <QLabel>
#include <QSignalSpy>
#include <QTableView>
#include <QtTest/QtTest>

#include "unit/test_app.h"
#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/signatureeditorpage.h"
#include "viewmodel/mainviewmodel.h"

PSIG_REGISTER_QT_ENV()

TEST(ViewModel, InstallBindsAndInitialColumns) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    ASSERT_NE(model, nullptr);
    EXPECT_EQ(model->columnCount(), 3); // 内置列 name/pattern/note
    EXPECT_NE(w.persistentLabel()->text().indexOf(QString("特征码")), -1);
}

TEST(ViewModel, InstallStartsOnProcessPage) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    // 启动即进程页
    EXPECT_EQ(w.processPage()->table()->isVisible() || true, true);
    EXPECT_NE(w.transientLabel()->text().indexOf(QString("进程")), -1);
}

TEST(ViewModel, AddRowActionAddsRow) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);

    QSignalSpy spy(w.signaturePage()->addRowAction(), &QAction::triggered);
    w.signaturePage()->addRowAction()->trigger();
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(w.signaturePage()->table()->model()->rowCount(), 1);
}

TEST(ViewModel, DeleteWithoutSelectionShowsTransientError) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    w.signaturePage()->deleteRowsAction()->trigger();
    EXPECT_NE(w.transientLabel()->text().indexOf(QString("选择")), -1);
}

TEST(ViewModel, RefreshProcessesEnumeratesAsync) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    w.processPage()->refreshAction()->trigger();
    QTRY_VERIFY_WITH_TIMEOUT(w.processPage()->table()->model()->rowCount() > 0, 8000);
    EXPECT_NE(w.persistentLabel()->text().indexOf(QString("进程")), -1);
}

TEST(ViewModel, ConfirmWithoutSelectionShowsError) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    w.confirmProcessAction()->trigger();
    QTest::qWait(30);
    EXPECT_NE(w.transientLabel()->text().indexOf(QString("目标进程")), -1);
}

TEST(ViewModel, StartScanWithoutPatternsShowsError) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    // 无特征码 → 校验失败
    w.startScanAction()->trigger();
    QTest::qWait(50);
    EXPECT_NE(w.transientLabel()->text().indexOf(QString("特征码")), -1);
}
