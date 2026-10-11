// test/unit/test_view.cpp - View 层单元测试（公开方法；不链接 ViewModel/静态库）
#include <gtest/gtest.h>

#include <QLabel>
#include <QPlainTextEdit>
#include <QTableView>

#include "model/signaturetablemodel.h"
#include "unit/test_app.h"
#include "view/columnmanagerdialog.h"
#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/resultpage.h"
#include "view/signatureeditorpage.h"

PSIG_REGISTER_QT_ENV()

TEST(View, MainWindowGetters) {
    MainWindow w;
    ASSERT_NE(w.signaturePage(), nullptr);
    ASSERT_NE(w.processPage(), nullptr);
    ASSERT_NE(w.resultPage(), nullptr);
    ASSERT_NE(w.importSignaturesAction(), nullptr);
    ASSERT_NE(w.startScanAction(), nullptr);
    ASSERT_NE(w.persistentLabel(), nullptr);
    ASSERT_NE(w.transientLabel(), nullptr);
    ASSERT_NE(w.logEdit(), nullptr);
}

TEST(View, SignatureEditorPageSetModel) {
    SignatureEditorPage page;
    SignatureTableModel model;
    model.setColumns({{"name", "名称", "text", true}});
    model.addRow();
    page.setModel(&model);
    ASSERT_NE(page.table()->model(), nullptr);
    EXPECT_EQ(page.table()->model()->rowCount(), 1);
    ASSERT_NE(page.addRowBtn(), nullptr);
}

TEST(View, ProcessSelectionPageMultiSelect) {
    ProcessSelectionPage page;
    EXPECT_EQ(page.table()->selectionMode(), QAbstractItemView::ExtendedSelection);
}

TEST(View, ResultPageReadOnly) {
    ResultPage page;
    EXPECT_EQ(page.table()->editTriggers(), QAbstractItemView::NoEditTriggers);
    ASSERT_NE(page.exportCsvBtn(), nullptr);
    ASSERT_NE(page.exportDbBtn(), nullptr);
    ASSERT_NE(page.exportTxtBtn(), nullptr);
}

TEST(View, StatusAndLog) {
    MainWindow w;
    w.setPersistentStatus("就绪 | 进程 0 | 特征码 0");
    EXPECT_EQ(w.persistentLabel()->text(), QString("就绪 | 进程 0 | 特征码 0"));
    w.setTransientMessage("hello", true);
    EXPECT_EQ(w.transientLabel()->text(), QString("hello"));
    w.appendLog("line1");
    w.appendLog("line2");
    EXPECT_NE(w.logEdit()->toPlainText().indexOf("line1"), -1);
    w.clearLog();
    EXPECT_TRUE(w.logEdit()->toPlainText().isEmpty());
}

TEST(View, ColumnManagerDialog) {
    ColumnManagerDialog dlg;
    dlg.setColumns({{"name", "名称", "text", true}, {"c1", "自定义", "text", false}});
    auto cols = dlg.columns();
    ASSERT_EQ(cols.size(), 2u);
    EXPECT_EQ(cols[1].title, QString("自定义"));
    ASSERT_NE(dlg.addBtn(), nullptr);
}
