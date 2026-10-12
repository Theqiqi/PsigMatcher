// test_signatures_exchange.cpp - 特征码表 .db/.csv 交换（含 v0.3.1 目标进程元数据）
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include "psig.h"

namespace {

// 构造一个最小特征码表（2 列 / 1 行）
psig::Table MakeTable(const std::string& targetProcess) {
    psig::Table t;
    psig::ColumnDef name;
    name.key = "name";
    name.title = "名称";
    name.type = "text";
    name.builtin = true;
    psig::ColumnDef pattern;
    pattern.key = "pattern";
    pattern.title = "特征码";
    pattern.type = "text";
    pattern.builtin = true;
    t.columns = {name, pattern};
    psig::TableRow r;
    r.cells = {"sig-a", "48 8B ?? 05"};
    t.rows = {r};
    t.targetProcess = targetProcess;
    return t;
}

std::string TempPath(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

} // namespace

// R-01：导出携带目标进程名，导入读回一致
TEST(SignaturesExchange, DbRoundTripCarriesTargetProcess) {
    const std::string path = TempPath("psig_v031_roundtrip.db");
    std::remove(path.c_str());
    std::string err;
    ASSERT_TRUE(psig::ExportSignaturesDb(path, MakeTable("notepad.exe"), err)) << err;

    const psig::Table in = psig::ImportSignaturesDb(path, err);
    EXPECT_TRUE(err.empty()) << err;
    EXPECT_EQ(in.targetProcess, "notepad.exe");
    ASSERT_EQ(in.columns.size(), 2u);
    ASSERT_EQ(in.rows.size(), 1u);
    EXPECT_EQ(in.rows[0].cells.size(), 2u);
    EXPECT_EQ(in.rows[0].cells[0], "sig-a");
    EXPECT_EQ(in.rows[0].cells[1], "48 8B ?? 05");
    std::remove(path.c_str());
}

// R-01：空目标进程名 → 不写键；导入得到空串且不报错（旧库兼容）
TEST(SignaturesExchange, DbEmptyTargetProcessYieldsEmpty) {
    const std::string path = TempPath("psig_v031_empty.db");
    std::remove(path.c_str());
    std::string err;
    ASSERT_TRUE(psig::ExportSignaturesDb(path, MakeTable(""), err)) << err;

    const psig::Table in = psig::ImportSignaturesDb(path, err);
    EXPECT_TRUE(err.empty()) << err;
    EXPECT_TRUE(in.targetProcess.empty());
    std::remove(path.c_str());
}

// R-01：不存在的文件 → 置 err 返回空表（错误路径）
TEST(SignaturesExchange, DbMissingFileReportsError) {
    std::string err;
    const psig::Table in = psig::ImportSignaturesDb(TempPath("psig_v031_does_not_exist.db"), err);
    EXPECT_FALSE(err.empty());
    EXPECT_TRUE(in.targetProcess.empty());
}

// CSV 往返不受影响（不携带目标进程元数据）
TEST(SignaturesExchange, CsvRoundTripUnchanged) {
    const std::string path = TempPath("psig_v031_roundtrip.csv");
    std::remove(path.c_str());
    std::string err;
    ASSERT_TRUE(psig::ExportSignaturesCsv(path, MakeTable("notepad.exe"), err)) << err;

    const psig::Table in = psig::ImportSignaturesCsv(path, err);
    EXPECT_TRUE(err.empty()) << err;
    ASSERT_EQ(in.columns.size(), 2u);
    ASSERT_EQ(in.rows.size(), 1u);
    EXPECT_EQ(in.rows[0].cells[0], "sig-a");
    std::remove(path.c_str());
}
