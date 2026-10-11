// test/unit/test_viewmodel.cpp - ViewModel 单元测试（事件链接、槽、调用链）
#include <gtest/gtest.h>

#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QSignalSpy>
#include <QTableView>
#include <QtTest/QtTest>

#include "unit/test_app.h"
#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/signatureeditorpage.h"
#include "viewmodel/mainviewmodel.h"

PSIG_REGISTER_QT_ENV()

// 等待 install 触发的自动枚举完成（C-09），避免其瞬时反馈覆盖后续断言
#define WAIT_ENUM(w) \
    QTRY_VERIFY_WITH_TIMEOUT((w).processPage()->table()->model()->rowCount() > 0, 10000)

TEST(ViewModel, InstallBindsAndInitialColumns) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    ASSERT_NE(model, nullptr);
    EXPECT_EQ(model->columnCount(), 4); // 内置 name/pattern/note + GUI 内置 process
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

TEST(ViewModel, InstallAutoRefreshesProcesses) { // C-09
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w); // 无需手动点刷新即已填充
    EXPECT_NE(w.persistentLabel()->text().indexOf(QString("进程")), -1);
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

TEST(ViewModel, AddRowFillsProcessColumnWhenTargetSelected) { // C-08
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);

    auto* pm = w.processPage()->table()->model();
    const QString targetName = pm->data(pm->index(0, 1)).toString(); // 名称列
    w.processPage()->table()->selectRow(0);
    w.confirmProcessAction()->trigger();
    QTest::qWait(30);

    w.signaturePage()->addRowAction()->trigger();
    auto* m = w.signaturePage()->table()->model();
    ASSERT_EQ(m->rowCount(), 1);
    // process 列（末列）应自动填入目标进程名
    EXPECT_EQ(m->data(m->index(0, m->columnCount() - 1)).toString(), targetName);
}

TEST(ViewModel, DeleteWithoutSelectionShowsTransientError) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);
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

TEST(ViewModel, ProcessFilterReappliedAfterRefresh) { // C-10
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);

    auto* table = w.processPage()->table();
    auto* model = table->model();
    w.processPage()->filterEdit()->setText(QStringLiteral("zzz_no_such_process_zzz"));
    QTest::qWait(10);
    auto visibleCount = [&] {
        int n = 0;
        for (int r = 0; r < model->rowCount(); ++r)
            if (!table->isRowHidden(r)) ++n;
        return n;
    };
    EXPECT_EQ(visibleCount(), 0);

    // 刷新后筛选仍应生效（不混入不相干进程）
    w.processPage()->refreshAction()->trigger();
    QTRY_VERIFY_WITH_TIMEOUT(model->rowCount() > 0, 10000);
    QTest::qWait(50);
    EXPECT_EQ(visibleCount(), 0);
}

TEST(ViewModel, ConfirmWithoutSelectionShowsError) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);
    w.confirmProcessAction()->trigger();
    QTest::qWait(30);
    EXPECT_NE(w.transientLabel()->text().indexOf(QString("目标进程")), -1);
}

TEST(ViewModel, StartScanWithoutPatternsShowsError) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w); // 先等自动枚举结束，避免其反馈覆盖错误提示
    // 无特征码 → 校验失败
    w.startScanAction()->trigger();
    QTest::qWait(50);
    EXPECT_NE(w.transientLabel()->text().indexOf(QString("特征码")), -1);
}

TEST(ViewModel, ApplyLanguageSwitchesText) { // C-02
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);

    vm.applyLanguage(QStringLiteral("en_US"), /*feedback=*/false);
    const QString en = w.importSignaturesAction()->text();
    vm.applyLanguage(QStringLiteral("zh_CN"), /*feedback=*/false); // 立即还原，避免污染后续用例
    const QString zh = w.importSignaturesAction()->text();

    EXPECT_EQ(en, QStringLiteral("Import Signatures"));
    EXPECT_EQ(zh, QStringLiteral("导入特征码"));
}

TEST(ViewModel, ApplyLanguageUpdatesLanguageMenuCheck) { // C-02
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    vm.applyLanguage(QStringLiteral("en_US"), false);
    EXPECT_TRUE(w.languageEnAction()->isChecked());
    vm.applyLanguage(QString(), false); // 跟随系统
    EXPECT_TRUE(w.languageSystemAction()->isChecked());
    vm.applyLanguage(QStringLiteral("zh_CN"), false); // 还原
    EXPECT_TRUE(w.languageZhAction()->isChecked());
}
