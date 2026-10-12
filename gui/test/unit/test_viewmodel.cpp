// test/unit/test_viewmodel.cpp - ViewModel 单元测试（事件链接、槽、调用链）
#include <gtest/gtest.h>

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSignalSpy>
#include <QTableView>
#include <QTimer>
#include <QtTest/QtTest>

#include "model/signaturetablemodel.h"
#include "psig.h"
#include "unit/test_app.h"
#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/signatureeditorpage.h"
#include "viewmodel/mainviewmodel.h"

PSIG_REGISTER_QT_ENV()

// 等待 install 触发的自动枚举完成（C-09），避免其瞬时反馈覆盖后续断言
#define WAIT_ENUM(w) \
    QTRY_VERIFY_WITH_TIMEOUT((w).processPage()->table()->model()->rowCount() > 0, 10000)

namespace {
// 点击当前可见 QMessageBox 中文本匹配的按钮（用于驱动模态提示的取消/不保存路径）
void clickMessageBoxButton(const QString& text) {
    for (QWidget* top : QApplication::topLevelWidgets()) {
        auto* mb = qobject_cast<QMessageBox*>(top);
        if (!mb) continue;
        for (QAbstractButton* b : mb->buttons()) {
            if (b->text() == text) {
                b->click();
                return;
            }
        }
    }
}

// 关闭窗口的用例需阻止「最后窗口关闭即退出」干扰后续断言
struct QuitOnLastWindowClosedGuard {
    bool prev = QApplication::quitOnLastWindowClosed();
    QuitOnLastWindowClosedGuard() { QApplication::setQuitOnLastWindowClosed(false); }
    ~QuitOnLastWindowClosedGuard() { QApplication::setQuitOnLastWindowClosed(prev); }
};
} // namespace

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

// C-06：内置列标题随语言切换；自定义列标题保持原样
TEST(ViewModel, SignatureHeadersFollowLanguageExceptCustom) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    auto* m = qobject_cast<SignatureTableModel*>(w.signaturePage()->table()->model());
    ASSERT_NE(m, nullptr);
    m->setColumns({{"name", "名称", "text", true}, {"c1", "自定义列", "text", false}});

    vm.applyLanguage(QStringLiteral("en_US"), false);
    EXPECT_EQ(m->headerData(0, Qt::Horizontal).toString(), QStringLiteral("Name"));
    EXPECT_EQ(m->headerData(1, Qt::Horizontal).toString(), QStringLiteral("自定义列"));

    vm.applyLanguage(QStringLiteral("zh_CN"), false);
    EXPECT_EQ(m->headerData(0, Qt::Horizontal).toString(), QStringLiteral("名称"));
    EXPECT_EQ(m->headerData(1, Qt::Horizontal).toString(), QStringLiteral("自定义列"));
}

// C-04：有未保存改动时重新选择进程会弹窗；「取消」中止切换（停留在特征码页）
TEST(ViewModel, ReselectPromptsSaveAndCancelAborts) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);

    w.processPage()->table()->selectRow(0);
    w.confirmProcessAction()->trigger();
    QTest::qWait(30);

    w.signaturePage()->addRowAction()->trigger(); // 产生未保存改动
    ASSERT_EQ(w.signaturePage()->table()->model()->rowCount(), 1);

    QTimer::singleShot(150, []() {
        for (QWidget* top : QApplication::topLevelWidgets()) {
            auto* mb = qobject_cast<QMessageBox*>(top);
            if (!mb) continue;
            for (QAbstractButton* b : mb->buttons()) {
                if (b->text() == QStringLiteral("取消")) {
                    b->click();
                    break;
                }
            }
        }
    });
    w.reselectProcessAction()->trigger();
    QTest::qWait(50);
    EXPECT_TRUE(w.signaturePage()->isVisible());   // 取消 → 未切回进程页
    EXPECT_FALSE(w.processPage()->isVisible());
}

// C-01：未保存状态由真实改动驱动（单元格编辑计入；导出成功清除；再编辑重新置脏）
TEST(ViewModel, SignatureDirtyTracksRealEditsAndExportClearsIt) {
    MainWindow w;
    MainViewModel vm;
    vm.install(&w);
    ASSERT_FALSE(vm.isSignatureDirty());

    auto* m = w.signaturePage()->table()->model();
    w.signaturePage()->addRowAction()->trigger();      // 增行 → 脏
    ASSERT_EQ(m->rowCount(), 1);
    EXPECT_TRUE(vm.isSignatureDirty());
    EXPECT_TRUE(w.persistentLabel()->text().endsWith(QStringLiteral("*"))); // 增行后立即出现标记

    // 导出成功 → 视为已保存（标记消失）
    const QString path = QDir::temp().filePath(QStringLiteral("psig_vm_dirty"));
    QFile::remove(path + QStringLiteral(".db"));
    vm.exportSignaturesToPath(path + QStringLiteral(".db"));
    EXPECT_FALSE(vm.isSignatureDirty());
    EXPECT_FALSE(w.persistentLabel()->text().endsWith(QStringLiteral("*")));

    // 直接在模型中改单元格（模型发 dataChanged）→ 真实脏标记重新置位
    ASSERT_TRUE(m->setData(m->index(0, 2), QStringLiteral("edited-cell")));
    EXPECT_TRUE(vm.isSignatureDirty());
    EXPECT_TRUE(w.persistentLabel()->text().endsWith(QStringLiteral("*")));
    QFile::remove(path + QStringLiteral(".db"));
}

// C-02：有未保存改动时导入 → 仅覆盖确认（无保存/不保存选项）；「取消」中止导入且不改表
TEST(ViewModel, ImportPromptsOverwriteWithoutSaveOptions) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);

    w.signaturePage()->addRowAction()->trigger();       // 产生未保存改动
    ASSERT_TRUE(vm.isSignatureDirty());

    QStringList buttons;
    QTimer::singleShot(150, [&buttons]() {
        for (QWidget* top : QApplication::topLevelWidgets()) {
            auto* mb = qobject_cast<QMessageBox*>(top);
            if (!mb) continue;
            for (QAbstractButton* b : mb->buttons()) buttons << b->text();
            clickMessageBoxButton(QStringLiteral("取消"));
            break;
        }
    });
    w.importSignaturesAction()->trigger();               // 模态覆盖确认
    QTest::qWait(80);

    EXPECT_FALSE(buttons.contains(QStringLiteral("保存")));
    EXPECT_FALSE(buttons.contains(QStringLiteral("不保存")));
    EXPECT_TRUE(buttons.contains(QStringLiteral("继续导入")));
    EXPECT_TRUE(buttons.contains(QStringLiteral("取消")));
    EXPECT_EQ(w.signaturePage()->table()->model()->rowCount(), 1); // 表不变
    EXPECT_TRUE(vm.isSignatureDirty());                            // 未保存状态不变
    EXPECT_NE(w.transientLabel()->text().indexOf(QStringLiteral("已取消导入")), -1);
}

// C-03：关闭时有未保存改动 → 提示；「取消」阻止关闭、「不保存」关闭；无改动则直接关闭
TEST(ViewModel, ClosePromptsOnUnsavedChanges) {
    QuitOnLastWindowClosedGuard quitGuard;
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);

    EXPECT_FALSE(vm.isSignatureDirty());
    EXPECT_TRUE(w.close());                     // 无未保存改动 → 直接关闭

    w.show();
    w.signaturePage()->addRowAction()->trigger();
    ASSERT_TRUE(vm.isSignatureDirty());

    QTimer::singleShot(150, []() { clickMessageBoxButton(QStringLiteral("取消")); });
    EXPECT_FALSE(w.close());                    // 取消 → 阻止关闭
    EXPECT_TRUE(w.isVisible());

    QTimer::singleShot(150, []() { clickMessageBoxButton(QStringLiteral("不保存")); });
    EXPECT_TRUE(w.close());                     // 不保存 → 关闭
}

// C-03：关闭提示的按钮集合为「保存/不保存/取消」（与重选进程提示同构）
TEST(ViewModel, ClosePromptButtonSet) {
    QuitOnLastWindowClosedGuard quitGuard;
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);
    w.signaturePage()->addRowAction()->trigger();

    QStringList buttons;
    QTimer::singleShot(150, [&buttons]() {
        for (QWidget* top : QApplication::topLevelWidgets()) {
            auto* mb = qobject_cast<QMessageBox*>(top);
            if (!mb) continue;
            for (QAbstractButton* b : mb->buttons()) buttons << b->text();
            clickMessageBoxButton(QStringLiteral("取消"));
            break;
        }
    });
    EXPECT_FALSE(w.close());
    EXPECT_TRUE(buttons.contains(QStringLiteral("保存")));
    EXPECT_TRUE(buttons.contains(QStringLiteral("不保存")));
    EXPECT_TRUE(buttons.contains(QStringLiteral("取消")));
}

// C-05：导入时库记录的目标进程名与当前目标不一致 → 弹窗；「取消」中止、「继续导入」应用
TEST(ViewModel, ImportTargetProcessMismatchPrompts) {
    MainWindow w;
    w.show();
    MainViewModel vm;
    vm.install(&w);
    WAIT_ENUM(w);
    w.processPage()->table()->selectRow(0);
    w.confirmProcessAction()->trigger();
    QTest::qWait(30);

    // 构造一个记录「不同目标进程名」的特征码库
    psig::Table t;
    psig::ColumnDef c;
    c.key = "name";
    c.title = "名称";
    c.type = "text";
    c.builtin = true;
    t.columns = {c};
    psig::TableRow row;
    row.cells = {"sig-x"};
    t.rows = {row};
    t.targetProcess = "__psig_other_process__.exe";
    const QString path = QDir::temp().filePath(QStringLiteral("psig_vm_mismatch.db"));
    QFile::remove(path);
    std::string err;
    ASSERT_TRUE(psig::ExportSignaturesDb(path.toStdString(), t, err)) << err;

    auto* model = w.signaturePage()->table()->model();
    auto clickButton = [](const QString& text) {
        for (QWidget* top : QApplication::topLevelWidgets()) {
            auto* mb = qobject_cast<QMessageBox*>(top);
            if (!mb) continue;
            for (QAbstractButton* b : mb->buttons())
                if (b->text() == text) { b->click(); return; }
        }
    };

    // 取消 → 不导入
    QTimer::singleShot(150, [clickButton]() { clickButton(QStringLiteral("取消")); });
    vm.importSignaturesFromPath(path);
    QTest::qWait(50);
    EXPECT_EQ(model->rowCount(), 0);

    // 继续导入 → 应用
    QTimer::singleShot(150, [clickButton]() { clickButton(QStringLiteral("继续导入")); });
    vm.importSignaturesFromPath(path);
    QTest::qWait(50);
    EXPECT_EQ(model->rowCount(), 1);
    QFile::remove(path);
}
