// test/unit/test_model.cpp - Model 层单元测试（data/setData）
#include <gtest/gtest.h>

#include <QIcon>
#include <QPixmap>

#include "model/processtablemodel.h"
#include "model/resulttablemodel.h"
#include "model/signaturetablemodel.h"
#include "unit/test_app.h"

PSIG_REGISTER_QT_ENV()

TEST(SignatureTableModel, ColumnsAndRows) {
    SignatureTableModel m;
    m.setColumns({{"name", "名称", "text", true}, {"pattern", "特征码", "text", true}});
    EXPECT_EQ(m.rowCount(), 0);
    EXPECT_EQ(m.columnCount(), 2);
    m.addRow();
    EXPECT_EQ(m.rowCount(), 1);
    EXPECT_EQ(m.headerData(0, Qt::Horizontal).toString(), QString("名称"));
}

TEST(SignatureTableModel, SetDataAndValueAt) {
    SignatureTableModel m;
    m.setColumns({{"name", "名称", "text", true}, {"pattern", "特征码", "text", true}});
    m.addRow();
    QModelIndex idx = m.index(0, 1);
    EXPECT_TRUE(m.setData(idx, "48 8B ?? 05", Qt::EditRole));
    EXPECT_EQ(m.data(idx, Qt::DisplayRole).toString(), QString("48 8B ?? 05"));
    EXPECT_EQ(m.valueAt(0, "pattern"), QString("48 8B ?? 05"));
}

TEST(SignatureTableModel, RemoveRows) {
    SignatureTableModel m;
    m.setColumns({{"name", "名称", "text", true}});
    m.addRow();
    m.addRow();
    m.addRow();
    ASSERT_EQ(m.rowCount(), 3);
    m.removeRows({0, 2});
    EXPECT_EQ(m.rowCount(), 1);
}

TEST(ProcessTableModel, SetAndQuery) {
    ProcessTableModel m;
    m.setProcesses({{1234, "a.exe", "C:\\a.exe", true}, {5678, "b.exe", "C:\\b.exe", false}});
    ASSERT_EQ(m.rowCount(), 2);
    EXPECT_EQ(m.data(m.index(0, 0)).toUInt(), 1234u);
    EXPECT_EQ(m.data(m.index(1, 3)).toString(), QString("否"));
    EXPECT_EQ(m.processAt(1).name, QString("b.exe"));
}

// C-03：空名进程（PID 0/4 等）显示兜底名，而非空白
TEST(ProcessTableModel, EmptyNameFallback) {
    ProcessTableModel m;
    m.setProcesses({{0, "", "", true}, {4, "", "", true}, {999, "", "", false}});
    EXPECT_EQ(m.data(m.index(0, 1)).toString(), QString("系统空闲进程"));
    EXPECT_EQ(m.data(m.index(1, 1)).toString(), QString("系统"));
    EXPECT_EQ(m.data(m.index(2, 1)).toString(), QString("未知进程"));
    // 有名进程不受影响
    m.setProcesses({{1234, QStringLiteral("a.exe"), QStringLiteral("C:\\a.exe"), true}});
    EXPECT_EQ(m.data(m.index(0, 1)).toString(), QString("a.exe"));
}

TEST(ProcessTableModel, ReadOnlyFlags) {
    ProcessTableModel m;
    m.setProcesses({{1, "x", "", true}});
    EXPECT_FALSE(m.flags(m.index(0, 0)) & Qt::ItemIsEditable);
}

TEST(ResultTableModel, SetAndFormat) {
    ResultTableModel m;
    ResultRow r;
    r.patternName = "p1";
    r.pid = 100;
    r.processName = "x.exe";
    r.address = 0x140001000;
    r.moduleName = "x.exe";
    r.moduleBase = 0x140000000;
    r.matchedBytes = "DE AD";
    r.hitTime = "2026-10-11T12:00:00";
    m.setResults({r});
    ASSERT_EQ(m.rowCount(), 1);
    EXPECT_EQ(m.columnCount(), 10);
    EXPECT_EQ(m.data(m.index(0, 0)).toString(), QString("p1"));
    EXPECT_EQ(m.data(m.index(0, 3)).toString(), QString("5368713216")); // 地址(十进制)
    EXPECT_EQ(m.data(m.index(0, 4)).toString(), QString("0x140001000")); // 地址(十六进制)
    m.clear();
    EXPECT_EQ(m.rowCount(), 0);
}

TEST(ResultTableModel, CustomColumnEditable) {
    ResultTableModel m;
    m.setResults({ResultRow{}});
    const int col = ResultTableModel::customColumn();
    EXPECT_EQ(col, 9);
    EXPECT_TRUE(m.flags(m.index(0, col)) & Qt::ItemIsEditable);
    EXPECT_FALSE(m.flags(m.index(0, 0)) & Qt::ItemIsEditable);
    EXPECT_TRUE(m.setData(m.index(0, col), QStringLiteral("备注"), Qt::EditRole));
    EXPECT_EQ(m.data(m.index(0, col)).toString(), QStringLiteral("备注"));
    // 非自定义列不可编辑
    EXPECT_FALSE(m.setData(m.index(0, 0), QStringLiteral("x"), Qt::EditRole));
}

// C-01：末列由「自定义」更名为「备注」
TEST(ResultTableModel, CustomColumnHeaderRenamed) {
    ResultTableModel m;
    m.setResults({ResultRow{}});
    EXPECT_EQ(m.headerData(ResultTableModel::customColumn(), Qt::Horizontal).toString(),
              QString("备注"));
}

TEST(ResultTableModel, RemoveAndSetCustom) {
    ResultTableModel m;
    m.setResults({ResultRow{}, ResultRow{}, ResultRow{}});
    ASSERT_EQ(m.rowCount(), 3);
    m.setCustom({0, 2}, QStringLiteral("批量"));
    EXPECT_EQ(m.data(m.index(0, ResultTableModel::customColumn())).toString(), QStringLiteral("批量"));
    EXPECT_EQ(m.data(m.index(2, ResultTableModel::customColumn())).toString(), QStringLiteral("批量"));
    m.removeRows({0, 2});
    EXPECT_EQ(m.rowCount(), 1);
}

// C-06 / C-08：新增多行 + 进程列默认值
TEST(SignatureTableModel, AddRowsWithProcessDefault) {
    SignatureTableModel m;
    m.setColumns({{"name", "名称", "text", true},
                  {"pattern", "特征码", "text", true},
                  {"note", "备注", "text", true},
                  {"process", "进程", "text", true}});
    m.addRows(3, QStringLiteral("notepad.exe"));
    ASSERT_EQ(m.rowCount(), 3);
    EXPECT_EQ(m.valueAt(0, "process"), QStringLiteral("notepad.exe"));
    EXPECT_EQ(m.valueAt(2, "process"), QStringLiteral("notepad.exe"));
    m.addRow(QStringLiteral("calc.exe"));
    EXPECT_EQ(m.valueAt(3, "process"), QStringLiteral("calc.exe"));

    // 无 process 列时：仅新增空行，不报错
    SignatureTableModel m2;
    m2.setColumns({{"name", "名称", "text", true}});
    m2.addRows(2, QStringLiteral("x"));
    EXPECT_EQ(m2.rowCount(), 2);
    EXPECT_EQ(m2.valueAt(0, "name"), QString());
}

// C-08：进程列存在时 valueAt 返回默认值；内置列标题经 tr() 提供
TEST(SignatureTableModel, BuiltinHeaderTitles) {
    SignatureTableModel m;
    m.setColumns({{"name", "名称", "text", true},
                  {"pattern", "特征码", "text", true},
                  {"process", "进程", "text", true},
                  {"c1", "自定义列", "text", false}});
    EXPECT_EQ(m.headerData(0, Qt::Horizontal).toString(), QString("名称"));
    EXPECT_EQ(m.headerData(1, Qt::Horizontal).toString(), QString("特征码"));
    EXPECT_EQ(m.headerData(2, Qt::Horizontal).toString(), QString("进程"));
    EXPECT_EQ(m.headerData(3, Qt::Horizontal).toString(), QString("自定义列")); // 自定义列沿用标题
}

// C-11：进程表按列排序
TEST(ProcessTableModel, SortByColumns) {
    ProcessTableModel m;
    m.setProcesses({{300, "c.exe", "C:\\c", true},
                    {100, "a.exe", "C:\\a", false},
                    {200, "b.exe", "C:\\b", true}});
    m.sort(0, Qt::AscendingOrder);
    EXPECT_EQ(m.processAt(0).pid, 100u);
    EXPECT_EQ(m.processAt(2).pid, 300u);

    m.sort(1, Qt::DescendingOrder);
    EXPECT_EQ(m.processAt(0).name, QString("c.exe"));

    m.sort(3, Qt::AscendingOrder); // 可访问：false 在前
    EXPECT_FALSE(m.processAt(0).accessible);
}

// C-12：名称列 DecorationRole 提供图标
TEST(ProcessTableModel, SetIconDecorationRole) {
    ProcessTableModel m;
    m.setProcesses({{1, "a.exe", "C:\\a", true}});
    // 初始：DecorationRole 承载 null 图标（QVariant 本身有效但图标为空）
    EXPECT_TRUE(m.data(m.index(0, 1), Qt::DecorationRole).value<QIcon>().isNull());

    QPixmap pm(16, 16);
    pm.fill(Qt::red);
    m.setIcon(0, QIcon(pm));
    const QVariant v = m.data(m.index(0, 1), Qt::DecorationRole);
    EXPECT_TRUE(v.isValid());
    EXPECT_FALSE(v.value<QIcon>().isNull());
    // 非名称列无 DecorationRole
    EXPECT_FALSE(m.data(m.index(0, 0), Qt::DecorationRole).isValid());
}

