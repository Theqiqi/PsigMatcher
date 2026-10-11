// test/unit/test_view.cpp - View 层单元测试（公开方法；不链接 ViewModel/静态库）
#include <gtest/gtest.h>

#include <QAction>
#include <QActionGroup>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QTableWidget>

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

// C-09 / C-11：进程页筛选框在底部、表格可排序
TEST(View, ProcessSelectionPageLayoutAndSorting) {
    ProcessSelectionPage page;
    EXPECT_TRUE(page.table()->isSortingEnabled());
    EXPECT_TRUE(page.table()->horizontalHeader()->sectionsClickable());
    auto* lay = page.layout();
    ASSERT_NE(lay, nullptr);
    EXPECT_GT(lay->indexOf(page.filterEdit()), lay->indexOf(page.table())); // 筛选框在表格下方
}

// C-01 / C-05：对话框按钮文案统一（本地化后为「确定/取消」）
TEST(View, DialogButtonTextsUnified) {
    ExportColumnsDialog e;
    ASSERT_NE(e.buttonBox(), nullptr);
    EXPECT_EQ(e.buttonBox()->button(QDialogButtonBox::Ok)->text(), QString("确定"));
    EXPECT_EQ(e.buttonBox()->button(QDialogButtonBox::Cancel)->text(), QString("取消"));

    AboutDialog a;
    ASSERT_NE(a.buttonBox(), nullptr);
    EXPECT_EQ(a.buttonBox()->button(QDialogButtonBox::Ok)->text(), QString("确定"));

    ColumnManagerDialog c;
    ASSERT_NE(c.buttonBox(), nullptr);
    EXPECT_EQ(c.buttonBox()->button(QDialogButtonBox::Ok)->text(), QString("确定"));
    EXPECT_EQ(c.buttonBox()->button(QDialogButtonBox::Cancel)->text(), QString("取消"));
}

// C-04：列管理对话框内部行操作（由 VM 接线的公开方法）
TEST(View, ColumnManagerDialogRowOperations) {
    ColumnManagerDialog dlg;
    dlg.setColumns({{QStringLiteral("name"), QStringLiteral("名称"), QStringLiteral("text"), true},
                    {QStringLiteral("c1"), QStringLiteral("自定义"), QStringLiteral("text"), false}});
    dlg.addColumnRow();
    ASSERT_EQ(dlg.columns().size(), 3u);
    EXPECT_EQ(dlg.columns()[2].title, QStringLiteral("新列"));
    dlg.table()->selectRow(2);
    dlg.moveSelectedUp();
    EXPECT_EQ(dlg.columns()[1].title, QStringLiteral("新列"));
    dlg.moveSelectedDown();
    EXPECT_EQ(dlg.columns()[2].title, QStringLiteral("新列"));
    dlg.table()->selectRow(2);
    dlg.removeSelectedColumnRow();
    EXPECT_EQ(dlg.columns().size(), 2u);
}

// 各页 retranslateUi 可重设文案（切换语言时由 MainWindow 调用）
TEST(View, PagesRetranslate) {
    SignatureEditorPage sp;
    sp.retranslateUi();
    EXPECT_EQ(sp.addRowsAction()->text(), QString("新增多行…"));

    ProcessSelectionPage pp;
    pp.retranslateUi();
    EXPECT_NE(pp.filterEdit()->placeholderText().indexOf(QString("过滤")), -1);

    ResultPage rp;
    rp.retranslateUi();
    EXPECT_EQ(rp.selectColumnsAction()->text(), QString("选择导出列…"));
}

// C-02：语言菜单动作存在且互斥（QActionGroup）
TEST(View, LanguageMenuActions) {
    MainWindow w;
    ASSERT_NE(w.languageZhAction(), nullptr);
    ASSERT_NE(w.languageEnAction(), nullptr);
    ASSERT_NE(w.languageSystemAction(), nullptr);
    ASSERT_NE(w.languageGroup(), nullptr);
    EXPECT_TRUE(w.languageZhAction()->isCheckable());
    EXPECT_EQ(w.languageGroup()->actions().size(), 3);
    w.setLanguageChecked(QStringLiteral("en_US"));
    EXPECT_TRUE(w.languageEnAction()->isChecked());
    w.setLanguageChecked(QString());
    EXPECT_TRUE(w.languageSystemAction()->isChecked());
}

