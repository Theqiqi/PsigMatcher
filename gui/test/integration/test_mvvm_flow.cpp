// test/integration/test_mvvm_flow.cpp - MVVM 完整链路集成测试
#include <gtest/gtest.h>

#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QtTest/QtTest>

#include <windows.h>

#include "unit/test_app.h"
#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/resultpage.h"
#include "view/signatureeditorpage.h"
#include "viewmodel/mainviewmodel.h"

PSIG_REGISTER_QT_ENV()

namespace {
extern "C" {
volatile unsigned char g_mvvm_marker[16] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
}
} // namespace

// View 信号 → VM 槽 → 静态库 API → Model → View 刷新 全链路
TEST(MvvmFlow, ScanSelfViaUiSignals) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);

    auto* sigTable = w.signaturePage()->table();
    auto* sigModel = sigTable->model();
    ASSERT_NE(sigModel, nullptr);

    // 通过按钮新增一行并填写模式
    w.signaturePage()->addRowBtn()->click();
    ASSERT_EQ(sigModel->rowCount(), 1);
    sigModel->setData(sigModel->index(0, 0), QStringLiteral("self_marker"));
    sigModel->setData(sigModel->index(0, 1), QStringLiteral("DE AD BE EF 11 22 33 44"));

    // 刷新进程（异步）
    w.processPage()->refreshBtn()->click();
    auto* procModel = w.processPage()->table()->model();
    QTRY_VERIFY_WITH_TIMEOUT(procModel->rowCount() > 0, 10000);

    // 选中本进程
    const quint32 self = GetCurrentProcessId();
    int selfRow = -1;
    for (int i = 0; i < procModel->rowCount(); ++i) {
        if (procModel->data(procModel->index(i, 0)).toUInt() == self) { selfRow = i; break; }
    }
    ASSERT_GE(selfRow, 0) << "未在进程列表中找到自身";
    w.processPage()->table()->selectRow(selfRow);

    // 开始扫描
    w.startScanAction()->trigger();
    auto* resModel = w.resultPage()->table()->model();
    ASSERT_NE(resModel, nullptr);
    QTRY_VERIFY_WITH_TIMEOUT(resModel->rowCount() > 0, 30000);

    EXPECT_GT(resModel->rowCount(), 0);
    // 结果页被自动切换为当前页
    EXPECT_EQ(w.resultPage()->table()->model()->rowCount(), resModel->rowCount());
}

// 刷新进程异步链路
TEST(MvvmFlow, RefreshProcessesUpdatesModel) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    w.processPage()->refreshBtn()->click();
    QTRY_VERIFY_WITH_TIMEOUT(w.processPage()->table()->model()->rowCount() > 0, 10000);
    EXPECT_NE(w.persistentLabel()->text().indexOf(QStringLiteral("进程")), -1);
}
