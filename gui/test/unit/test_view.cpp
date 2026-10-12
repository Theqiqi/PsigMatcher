// test/unit/test_view.cpp - View 层单元测试（公开方法；不链接 ViewModel/静态库）
#include <gtest/gtest.h>

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QImage>
#include <QMenu>
#include <QMenuBar>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QTableWidget>

#include "model/processtablemodel.h"
#include "model/resulttablemodel.h"
#include "model/signaturetablemodel.h"
#include "unit/test_app.h"
#include "view/aboutdialog.h"
#include "view/columnmanagerdialog.h"
#include "view/exportcolumnsdialog.h"
#include "view/hoverrowtableview.h"
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

// C-02：列管理「键 / 类型」改为下拉选择（非自由输入），且取值反映下拉当前项
TEST(View, ColumnManagerDialogKeyTypeCombos) {
    ColumnManagerDialog dlg;
    dlg.setKeyOptions({QStringLiteral("name"), QStringLiteral("pattern"), QStringLiteral("note")});
    dlg.setColumns({{QStringLiteral("name"), QStringLiteral("名称"), QStringLiteral("text"), true},
                    {QStringLiteral("pattern"), QStringLiteral("特征码"), QStringLiteral("number"), true}});
    auto* kc = qobject_cast<QComboBox*>(dlg.table()->cellWidget(0, 1));
    auto* tc = qobject_cast<QComboBox*>(dlg.table()->cellWidget(0, 2));
    ASSERT_NE(kc, nullptr);
    ASSERT_NE(tc, nullptr);
    EXPECT_EQ(kc->currentText(), QStringLiteral("name"));
    EXPECT_EQ(dlg.columns()[1].type, QStringLiteral("number"));

    // 改下拉 → columns() 反映新值
    kc->setCurrentText(QStringLiteral("note"));
    tc->setCurrentIndex(tc->findData(QStringLiteral("number")));
    auto cols = dlg.columns();
    EXPECT_EQ(cols[0].key, QStringLiteral("note"));
    EXPECT_EQ(cols[0].type, QStringLiteral("number"));
    EXPECT_EQ(cols[0].title, QStringLiteral("名称"));

    // 自定义键（不在内置选项中）不丢失
    dlg.setColumns({{QStringLiteral("c1"), QStringLiteral("自定义列"), QStringLiteral("text"), false}});
    auto* kc1 = qobject_cast<QComboBox*>(dlg.table()->cellWidget(0, 1));
    ASSERT_NE(kc1, nullptr);
    EXPECT_EQ(kc1->currentText(), QStringLiteral("c1"));
    EXPECT_EQ(dlg.columns()[0].key, QStringLiteral("c1"));
}

// C-07：进程列默认宽度（名称 = PID×2、可访问 = PID、路径占剩余）
TEST(View, ProcessSelectionPageDefaultColumnWidths) {
    ProcessTableModel model;
    model.setProcesses({{1, QStringLiteral("a.exe"), QStringLiteral("C:\\a.exe"), true}});
    ProcessSelectionPage page;
    page.setModel(&model);
    page.setDefaultColumnWidths();
    auto* t = page.table();
    EXPECT_EQ(t->columnWidth(1), t->columnWidth(0) * 2);
    EXPECT_EQ(t->columnWidth(3), t->columnWidth(0));
    EXPECT_EQ(t->horizontalHeader()->sectionResizeMode(2), QHeaderView::Stretch);
    // 仍可手动调整：非 Stretch 列保持 Interactive
    EXPECT_EQ(t->horizontalHeader()->sectionResizeMode(1), QHeaderView::Interactive);
}

// C-08：视图菜单含日志开关；「清空日志」只在日志窗口右键菜单
TEST(View, LogToggleInViewMenuAndClearInContextMenu) {
    MainWindow w;
    w.show(); // dock 可见性依赖主窗口已显示
    ASSERT_NE(w.toggleLogAction(), nullptr);
    EXPECT_TRUE(w.toggleLogAction()->isCheckable());
    EXPECT_EQ(w.toggleLogAction()->text(), QString("日志"));

    // 清空日志不在任何菜单栏菜单中
    bool inMenuBar = false;
    for (QMenu* menu : w.menuBar()->findChildren<QMenu*>()) {
        if (menu->actions().contains(w.clearLogAction())) inMenuBar = true;
    }
    EXPECT_FALSE(inMenuBar);
    // 日志开关在菜单栏中
    bool toggleInMenuBar = false;
    for (QMenu* menu : w.menuBar()->findChildren<QMenu*>()) {
        if (menu->actions().contains(w.toggleLogAction())) toggleInMenuBar = true;
    }
    EXPECT_TRUE(toggleInMenuBar);

    // 日志窗口右键菜单
    EXPECT_EQ(w.logEdit()->contextMenuPolicy(), Qt::CustomContextMenu);
    ASSERT_NE(w.logContextMenu(), nullptr);
    EXPECT_TRUE(w.logContextMenu()->actions().contains(w.clearLogAction()));

    // 显隐控制
    w.setLogDockVisible(false);
    EXPECT_FALSE(w.logDockVisible());
    w.setToggleLogChecked(true);
    EXPECT_TRUE(w.toggleLogAction()->isChecked());
    w.setLogDockVisible(true);
    EXPECT_TRUE(w.logDockVisible());
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

// C-04：进程页禁用角落按钮（点击左上角空白不再全选）
TEST(View, ProcessPageCornerButtonDisabled) {
    ProcessSelectionPage page;
    EXPECT_FALSE(page.table()->isCornerButtonEnabled());
    // 其余两页保留角落按钮
    SignatureEditorPage sp;
    EXPECT_TRUE(sp.table()->isCornerButtonEnabled());
    ResultPage rp;
    EXPECT_TRUE(rp.table()->isCornerButtonEnabled());
}

// C-01/C-02/C-03：整行悬停高亮——各单元格在自身矩形内绘制，不覆盖相邻列/不影响其他行
TEST(View, HoverRowHighlightsWholeRowWithinCells) {
    HoverRowTableView view;
    QStandardItemModel model(3, 2); // 空单元格，便于按像素判断底色
    view.setModel(&model);
    view.setAlternatingRowColors(true);
    view.resize(320, 200);
    view.setStyleSheet(QStringLiteral(
        "QTableView{background:#ffffff;alternate-background-color:#f4f7fb;"
        "gridline-color:#e2e8f0;selection-background-color:#3a6ea5;selection-color:#ffffff;}"));
    view.show();
    QApplication::processEvents();

    const QPoint vp = view.viewport()->pos();
    const QColor hover(0xE8, 0xF0, 0xF8);
    auto center = [&](int r, int c) {
        return view.visualRect(model.index(r, c)).center() + vp;
    };
    auto moveTo = [&](const QPoint& p) {
        QMouseEvent me(QEvent::MouseMove, QPointF(p), view.viewport()->mapToGlobal(p),
                       Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &me);
        QApplication::processEvents();
    };
    auto hoverAt = [&](int r, int c) { moveTo(view.visualRect(model.index(r, c)).center()); };

    // 基线：鼠标移到所有行下方空白（hoveredRow = -1，无悬停）
    const QPoint below(view.visualRect(model.index(2, 0)).center().x(),
                       view.viewport()->height() - 3);
    moveTo(below);
    ASSERT_EQ(view.hoveredRow(), -1);
    const QImage base = view.grab().toImage();
    ASSERT_NE(base.pixelColor(center(1, 0)).rgb(), hover.rgb());

    // 悬停到第 1 行
    hoverAt(1, 0);
    ASSERT_EQ(view.hoveredRow(), 1);
    const QImage hov = view.grab().toImage();
    // 悬停整行：列 0 与列 1 均为悬停色（各单元格各画自身矩形，不越界）
    EXPECT_EQ(hov.pixelColor(center(1, 0)).rgb(), hover.rgb());
    EXPECT_EQ(hov.pixelColor(center(1, 1)).rgb(), hover.rgb());
    // 其他行不受影响（未悬停行像素与基线一致，未被越界覆盖）
    EXPECT_EQ(hov.pixelColor(center(0, 0)).rgb(), base.pixelColor(center(0, 0)).rgb());
    EXPECT_EQ(hov.pixelColor(center(2, 0)).rgb(), base.pixelColor(center(2, 0)).rgb());

    // 鼠标移回行下方空白：悬停清空，第 1 行恢复
    moveTo(below);
    EXPECT_EQ(view.hoveredRow(), -1);
    const QImage after = view.grab().toImage();
    EXPECT_NE(after.pixelColor(center(1, 0)).rgb(), hover.rgb());
}

