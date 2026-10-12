// test/integration/test_user_flow.cpp - 用户流程测试（用户视角完整业务流程，真实数据）
#include <gtest/gtest.h>

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QtTest/QtTest>

#include <windows.h>

#include "model/resulttablemodel.h"
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

// 走通「新增特征码 → 刷新进程 → 选自身 → 扫描」并等待结果
bool scanSelf(MainWindow& w, MainViewModel& vm, const QString& name) {
    auto* sigModel = w.signaturePage()->table()->model();
    w.signaturePage()->addRowAction()->trigger();
    const int r = sigModel->rowCount() - 1;
    sigModel->setData(sigModel->index(r, 0), name);
    sigModel->setData(sigModel->index(r, 1), QStringLiteral("DE AD BE EF 11 22 33 44"));

    w.processPage()->refreshAction()->trigger();
    auto* procModel = w.processPage()->table()->model();
    if (!QTest::qWaitFor([&] { return procModel->rowCount() > 0; }, 10000)) return false;
    const quint32 self = GetCurrentProcessId();
    int selfRow = -1;
    for (int i = 0; i < procModel->rowCount(); ++i) {
        if (procModel->data(procModel->index(i, 0)).toUInt() == self) { selfRow = i; break; }
    }
    if (selfRow < 0) return false;
    w.processPage()->table()->selectRow(selfRow);

    w.startScanAction()->trigger();
    auto* resModel = w.resultPage()->table()->model();
    return QTest::qWaitFor([&] { return resModel->rowCount() > 0; }, 30000);
}
} // namespace

// UF-A4：新增行、多选删除（右键菜单动作）
TEST(UserFlow, AddAndDeleteRows) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    w.signaturePage()->addRowAction()->trigger();
    w.signaturePage()->addRowAction()->trigger();
    ASSERT_EQ(model->rowCount(), 2);
    w.signaturePage()->table()->selectAll();
    w.signaturePage()->deleteRowsAction()->trigger();
    EXPECT_EQ(model->rowCount(), 0);
}

// UF-A7：模板文本随特征码 CSV 往返携带
// C-04：模板文本与特征码表解耦——导出不带模板，导入不覆盖结果页模板
TEST(UserFlow, TemplateDecoupledFromSignatureTable) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    w.signaturePage()->addRowAction()->trigger();
    model->setData(model->index(0, 0), QStringLiteral("p1"));
    model->setData(model->index(0, 1), QStringLiteral("48 8B ?? 05"));
    vm.setTemplateText(QStringLiteral("模板第一行\n含,逗号"));

    const QString path = tempPath("psig_uf_tmpl.csv");
    QFile::remove(path);
    vm.exportSignaturesToPath(path);
    ASSERT_TRUE(QFileInfo::exists(path));

    // 导出文件不应包含结果页模板文本
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QString content = QString::fromUtf8(f.readAll());
    f.close();
    EXPECT_FALSE(content.contains(QStringLiteral("模板第一行")));

    // 导入不覆盖结果页当前模板
    w.resultPage()->setTemplateText(QStringLiteral("当前模板"));
    w.signaturePage()->table()->selectAll();
    w.signaturePage()->deleteRowsAction()->trigger();
    ASSERT_EQ(model->rowCount(), 0);
    vm.importSignaturesFromPath(path);
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_EQ(w.resultPage()->templateText(), QStringLiteral("当前模板"));
    QFile::remove(path);
}

// UF-03 / UF-01：导出特征码 CSV 再导入（往返一致）
TEST(UserFlow, ExportThenImportCsv) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    w.signaturePage()->addRowAction()->trigger();
    model->setData(model->index(0, 0), QStringLiteral("p1"));
    model->setData(model->index(0, 1), QStringLiteral("48 8B ?? 05"));

    const QString path = tempPath("psig_uf_sig.csv");
    QFile::remove(path);
    vm.exportSignaturesToPath(path);
    ASSERT_TRUE(QFileInfo::exists(path));
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("已导出")), -1);

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
    w.signaturePage()->addRowAction()->trigger();
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

// UF-14/UF-15：未保存状态贯通（真实文件）——编辑置脏、导出清脏、导入覆盖后清脏
TEST(UserFlow, UnsavedStateLifecycleWithRealFiles) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    ASSERT_FALSE(vm.isSignatureDirty());

    w.signaturePage()->addRowAction()->trigger();
    model->setData(model->index(0, 0), QStringLiteral("p1"));
    model->setData(model->index(0, 1), QStringLiteral("DE AD BE EF"));
    EXPECT_TRUE(vm.isSignatureDirty());

    const QString path = tempPath("psig_uf_unsaved.db");
    QFile::remove(path);
    vm.exportSignaturesToPath(path);
    ASSERT_TRUE(QFileInfo::exists(path));
    EXPECT_FALSE(vm.isSignatureDirty());           // 导出成功 = 已保存

    model->setData(model->index(0, 0), QStringLiteral("p1-edited"));
    EXPECT_TRUE(vm.isSignatureDirty());            // 单元格编辑重新置脏

    vm.importSignaturesFromPath(path);             // 导入覆盖 → 清脏
    EXPECT_FALSE(vm.isSignatureDirty());
    EXPECT_EQ(model->data(model->index(0, 0)).toString(), QStringLiteral("p1"));
    QFile::remove(path);
}

// UF-A10 错误路径：导入不存在的文件 → 瞬时错误反馈
TEST(UserFlow, ImportMissingFileShowsError) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    vm.importSignaturesFromPath(tempPath("no_such_file_xyz.csv"));
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("失败")), -1);
}

// UF-A2：确认选择进程后锁定并跳转特征码页
TEST(UserFlow, ConfirmProcessLocksAndNavigates) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    w.processPage()->refreshAction()->trigger();
    auto* procModel = w.processPage()->table()->model();
    QTRY_VERIFY_WITH_TIMEOUT(procModel->rowCount() > 0, 10000);
    w.processPage()->table()->selectRow(0);
    w.confirmProcessAction()->trigger();
    QTest::qWait(30);
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("已选定目标")), -1);
    EXPECT_NE(w.persistentLabel()->text().indexOf(QStringLiteral("目标")), -1);
}

// UF-A3：重新选择进程回到进程页
TEST(UserFlow, ReselectProcessReturnsToProcessPage) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    // 先等启动自动枚举结束，避免其瞬时反馈覆盖断言（C-09）
    QTRY_VERIFY_WITH_TIMEOUT(w.processPage()->table()->model()->rowCount() > 0, 10000);
    w.reselectProcessAction()->trigger();
    QTest::qWait(30);
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("重新选择")), -1);
}

// C-09：启动自动枚举进程（无需手动刷新）
TEST(UserFlow, StartupAutoEnumeratesProcesses) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    QTRY_VERIFY_WITH_TIMEOUT(w.processPage()->table()->model()->rowCount() > 0, 10000);
}

// C-10：筛选保持——刷新后只显示匹配项
TEST(UserFlow, FilterPersistsAcrossRefresh) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    auto* table = w.processPage()->table();
    auto* model = table->model();
    QTRY_VERIFY_WITH_TIMEOUT(model->rowCount() > 0, 10000);

    w.processPage()->filterEdit()->setText(QStringLiteral("zzz_nomatch_zzz"));
    QTest::qWait(10);
    w.processPage()->refreshAction()->trigger();
    QTRY_VERIFY_WITH_TIMEOUT(model->rowCount() > 0, 10000);
    QTest::qWait(50);

    int visible = 0;
    for (int r = 0; r < model->rowCount(); ++r)
        if (!table->isRowHidden(r)) ++visible;
    EXPECT_EQ(visible, 0);
}

// C-03：导出 CSV 带 UTF-8 BOM（中文表头在 Excel 中不乱码）
TEST(UserFlow, ExportedCsvHasUtf8Bom) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* model = w.signaturePage()->table()->model();
    w.signaturePage()->addRowAction()->trigger();
    model->setData(model->index(0, 1), QStringLiteral("AA BB"));

    const QString path = tempPath("psig_uf_bom.csv");
    QFile::remove(path);
    vm.exportSignaturesToPath(path);

    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QByteArray head = f.read(3);
    f.close();
    ASSERT_EQ(head.size(), 3);
    EXPECT_EQ(static_cast<unsigned char>(head[0]), 0xEFu);
    EXPECT_EQ(static_cast<unsigned char>(head[1]), 0xBBu);
    EXPECT_EQ(static_cast<unsigned char>(head[2]), 0xBFu);
    QFile::remove(path);
}

// UF-A5/A6：扫描后结果行删除 + 末列自定义编辑 + 三种格式导出
TEST(UserFlow, ScanThenEditAndExportResults) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    ASSERT_TRUE(scanSelf(w, vm, QStringLiteral("flow_marker")));

    auto* resModel = w.resultPage()->table()->model();
    const int before = resModel->rowCount();
    ASSERT_GT(before, 0);
    const int customCol = ResultTableModel::customColumn();

    // 末列可编辑
    resModel->setData(resModel->index(0, customCol), QStringLiteral("备注A"));
    EXPECT_EQ(resModel->index(0, customCol).data().toString(), QStringLiteral("备注A"));

    // 两种格式导出（结果仅 CSV/TXT，不再有 DB 入口）
    const QString csv = tempPath("psig_uf_res.csv");
    const QString txt = tempPath("psig_uf_res.txt");
    QFile::remove(csv);
    QFile::remove(txt);
    vm.exportResultsToPath(csv);
    vm.exportResultsToPath(txt);
    EXPECT_TRUE(QFileInfo::exists(csv));
    EXPECT_TRUE(QFileInfo::exists(txt));
    EXPECT_GT(QFileInfo(txt).size(), 0);

    // 删除第一行
    w.resultPage()->table()->selectRow(0);
    w.resultPage()->deleteRowsAction()->trigger();
    EXPECT_EQ(resModel->rowCount(), before - 1);

    QFile::remove(csv);
    QFile::remove(txt);
}

// UF-A8：结果导出携带模板 + 自定义列 + 选列子集
TEST(UserFlow, ExportResultsCarriesTemplateCustomColumns) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    ASSERT_TRUE(scanSelf(w, vm, QStringLiteral("flow_marker")));

    auto* resModel = w.resultPage()->table()->model();
    resModel->setData(resModel->index(0, ResultTableModel::customColumn()), QStringLiteral("我的备注"));
    vm.setTemplateText(QStringLiteral("结果模板"));
    vm.setExportColumns({QStringLiteral("pid"), QStringLiteral("custom")});

    const QString csv = tempPath("psig_uf_res_opt.csv");
    QFile::remove(csv);
    vm.exportResultsToPath(csv);
    ASSERT_TRUE(QFileInfo::exists(csv));

    QFile f(csv);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QString content = QString::fromUtf8(f.readAll());
    f.close();
    EXPECT_TRUE(content.contains(QStringLiteral("#psig-template")));
    EXPECT_TRUE(content.contains(QStringLiteral("结果模板")));
    EXPECT_TRUE(content.contains(QStringLiteral("我的备注")));
    EXPECT_TRUE(content.contains(QStringLiteral("PID")));
    EXPECT_FALSE(content.contains(QStringLiteral("特征码名"))); // 未选列不导出
    QFile::remove(csv);
}

// UF-12 错误路径：无结果时导出 → 瞬时错误
TEST(UserFlow, ExportResultsWithoutDataShowsError) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    vm.exportResultsToPath(tempPath("psig_uf_empty.csv"));
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("无结果")), -1);
}
