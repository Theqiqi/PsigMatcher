// test/unit/test_view.cpp - View 层单元测试（公开方法；不链接 ViewModel/静态库）
#include <gtest/gtest.h>

#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QTableView>

#include "model/resulttablemodel.h"
#include "model/signaturetablemodel.h"
#include "unit/test_app.h"
#include "view/aboutdialog.h"
#include "view/columnmanagerdialog.h"
#include "view/exportcolumnsdialog.h"
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
    ASSERT_NE(w.confirmProcessAction(), nullptr);
    ASSERT_NE(w.reselectProcessAction(), nullptr);
    ASSERT_NE(w.persistentLabel(), nullptr);
    ASSERT_NE(w.transientLabel(), nullptr);
    ASSERT_NE(w.logEdit(), nullptr);
}

TEST(View, SignatureEditorPageContextMenu) {
    SignatureEditorPage page;
    SignatureTableModel model;
    model.setColumns({{"name", "名称", "text", true}});
    model.addRow();
    page.setModel(&model);
    ASSERT_NE(page.table()->model(), nullptr);
    EXPECT_EQ(page.table()->model()->rowCount(), 1);
    // 行操作改由右键菜单提供
    ASSERT_NE(page.contextMenu(), nullptr);
    ASSERT_NE(page.addRowAction(), nullptr);
    ASSERT_NE(page.deleteRowsAction(), nullptr);
    ASSERT_NE(page.manageColumnsAction(), nullptr);
    EXPECT_EQ(page.table()->contextMenuPolicy(), Qt::CustomContextMenu);
}

TEST(View, ProcessSelectionPageMultiSelect) {
    ProcessSelectionPage page;
    EXPECT_EQ(page.table()->selectionMode(), QAbstractItemView::ExtendedSelection);
    ASSERT_NE(page.contextMenu(), nullptr);
    ASSERT_NE(page.useAsTargetAction(), nullptr);
    ASSERT_NE(page.refreshAction(), nullptr);
    ASSERT_NE(page.selectAllAction(), nullptr);
}

TEST(View, ResultPageContextMenuAndTemplate) {
    ResultPage page;
    ASSERT_NE(page.contextMenu(), nullptr);
    ASSERT_NE(page.exportCsvAction(), nullptr);
    ASSERT_NE(page.exportDbAction(), nullptr);
    ASSERT_NE(page.exportTxtAction(), nullptr);
    ASSERT_NE(page.selectColumnsAction(), nullptr);
    ASSERT_NE(page.deleteRowsAction(), nullptr);
    ASSERT_NE(page.fillCustomAction(), nullptr);
    ASSERT_NE(page.clearAction(), nullptr);
    ASSERT_NE(page.templateEdit(), nullptr);
    page.setTemplateText(QStringLiteral("tpl"));
    EXPECT_EQ(page.templateText(), QStringLiteral("tpl"));
    EXPECT_EQ(page.table()->contextMenuPolicy(), Qt::CustomContextMenu);
}

TEST(View, ResultTableCustomColumnEditable) {
    ResultPage page;
    ResultTableModel model;
    ResultRow r;
    r.patternName = QStringLiteral("p");
    model.setResults({r});
    page.setModel(&model);
    EXPECT_EQ(model.columnCount(), 10);
    const auto custom = model.flags(model.index(0, ResultTableModel::customColumn()));
    EXPECT_TRUE(custom & Qt::ItemIsEditable);
    const auto first = model.flags(model.index(0, 0));
    EXPECT_FALSE(first & Qt::ItemIsEditable);
    // 十进制地址列（第 3 列）与十六进制地址列（第 4 列）
    EXPECT_EQ(model.index(0, 3).data().toString(), QStringLiteral("0"));
    EXPECT_EQ(model.index(0, 9).data().toString(), QString());
}

TEST(View, StatusAndLog) {
    MainWindow w;
    w.setPersistentStatus("就绪 | 目标 - | 进程 0 | 特征码 0");
    EXPECT_EQ(w.persistentLabel()->text(), QString("就绪 | 目标 - | 进程 0 | 特征码 0"));
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

TEST(View, AboutDialog) {
    AboutDialog dlg;
    dlg.setInfo(QStringLiteral("进程特征码批量匹配器 v0.2.0"));
    EXPECT_EQ(dlg.info(), QStringLiteral("进程特征码批量匹配器 v0.2.0"));
    ASSERT_NE(dlg.buttonBox(), nullptr);
}

TEST(View, ExportColumnsDialog) {
    ExportColumnsDialog dlg;
    dlg.setColumns({{QStringLiteral("pid"), QStringLiteral("PID")},
                    {QStringLiteral("custom"), QStringLiteral("自定义")}},
                   {QStringLiteral("custom")});
    ASSERT_NE(dlg.list(), nullptr);
    EXPECT_EQ(dlg.list()->count(), 2);
    EXPECT_EQ(dlg.selectedKeys(), QStringList{QStringLiteral("custom")});
}
