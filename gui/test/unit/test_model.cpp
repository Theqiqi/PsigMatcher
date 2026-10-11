// test/unit/test_model.cpp - Model 层单元测试（data/setData）
#include <gtest/gtest.h>

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
    EXPECT_EQ(m.data(m.index(0, 0)).toString(), QString("p1"));
    EXPECT_EQ(m.data(m.index(0, 3)).toString(), QString("0x140001000"));
    m.clear();
    EXPECT_EQ(m.rowCount(), 0);
}
