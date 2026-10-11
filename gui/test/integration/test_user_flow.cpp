// test/integration/test_user_flow.cpp - 用户流程测试（用户视角完整业务流程，真实数据）
#include <gtest/gtest.h>

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
volatile unsigned char g_flow_marker[16] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
}

QString tempPath(const QString& name) { return QDir::tempPath() + QLatin1Char('/') + name; }
} // namespace

// UF-05 / UF-06：新增行、多选删除
TEST(UserFlow, AddAndDeleteRows) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    w.signaturePage()->addRowBtn()->click();
    w.signaturePage()->addRowBtn()->click();
    ASSERT_EQ(model->rowCount(), 2);
    w.signaturePage()->table()->selectAll();
    w.signaturePage()->deleteRowBtn()->click();
    EXPECT_EQ(model->rowCount(), 0);
}

// UF-03 / UF-01：导出特征码 CSV 再导入（往返一致）
TEST(UserFlow, ExportThenImportCsv) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    w.signaturePage()->addRowBtn()->click();
    model->setData(model->index(0, 0), QStringLiteral("p1"));
    model->setData(model->index(0, 1), QStringLiteral("48 8B ?? 05"));

    const QString path = tempPath("psig_uf_sig.csv");
    QFile::remove(path);
    vm.exportSignaturesToPath(path);
    ASSERT_TRUE(QFileInfo::exists(path));
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("已导出")), -1);

    // 导入回来
    w.signaturePage()->deleteRowBtn()->click(); // 无选择 → 仅提示，不清空
    vm.importSignaturesFromPath(path);
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_EQ(model->data(model->index(0, 1)).toString(), QStringLiteral("48 8B ?? 05"));
    QFile::remove(path);
}

// UF-02 / UF-03：导出特征码 DB 再导入
TEST(UserFlow, ExportThenImportDb) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    w.signaturePage()->addRowBtn()->click();
    model->setData(model->index(0, 0), QStringLiteral("p1"));
    model->setData(model->index(0, 1), QStringLiteral("DE AD BE EF"));

    const QString path = tempPath("psig_uf_sig.db");
    QFile::remove(path);
    vm.exportSignaturesToPath(path);
    ASSERT_TRUE(QFileInfo::exists(path));
    vm.importSignaturesFromPath(path);
    EXPECT_EQ(model->rowCount(), 1);
    QFile::remove(path);
}

// UF-01 错误路径：导入不存在的文件 → 瞬时错误反馈
TEST(UserFlow, ImportMissingFileShowsError) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    vm.importSignaturesFromPath(tempPath("no_such_file_xyz.csv"));
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("失败")), -1);
}

// UF-10 / UF-12 / UF-13 / UF-14：扫描自身后导出结果三种格式
TEST(UserFlow, ScanThenExportResultsThreeFormats) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    auto* sigModel = w.signaturePage()->table()->model();
    w.signaturePage()->addRowBtn()->click();
    sigModel->setData(sigModel->index(0, 0), QStringLiteral("flow_marker"));
    sigModel->setData(sigModel->index(0, 1), QStringLiteral("DE AD BE EF 11 22 33 44"));

    w.processPage()->refreshBtn()->click();
    auto* procModel = w.processPage()->table()->model();
    QTRY_VERIFY_WITH_TIMEOUT(procModel->rowCount() > 0, 10000);
    const quint32 self = GetCurrentProcessId();
    int selfRow = -1;
    for (int i = 0; i < procModel->rowCount(); ++i) {
        if (procModel->data(procModel->index(i, 0)).toUInt() == self) { selfRow = i; break; }
    }
    ASSERT_GE(selfRow, 0);
    w.processPage()->table()->selectRow(selfRow);

    w.startScanAction()->trigger();
    auto* resModel = w.resultPage()->table()->model();
    QTRY_VERIFY_WITH_TIMEOUT(resModel->rowCount() > 0, 30000);

    const QString csv = tempPath("psig_uf_res.csv");
    const QString db = tempPath("psig_uf_res.db");
    const QString txt = tempPath("psig_uf_res.txt");
    QFile::remove(csv);
    QFile::remove(db);
    QFile::remove(txt);
    vm.exportResultsToPath(csv);
    vm.exportResultsToPath(db);
    vm.exportResultsToPath(txt);
    EXPECT_TRUE(QFileInfo::exists(csv));
    EXPECT_TRUE(QFileInfo::exists(db));
    EXPECT_TRUE(QFileInfo::exists(txt));
    EXPECT_GT(QFileInfo(txt).size(), 0);
    QFile::remove(csv);
    QFile::remove(db);
    QFile::remove(txt);
}

// UF-12 错误路径：无结果时导出 → 瞬时错误
TEST(UserFlow, ExportResultsWithoutDataShowsError) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    vm.exportResultsToPath(tempPath("psig_uf_empty.csv"));
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("无结果")), -1);
}
