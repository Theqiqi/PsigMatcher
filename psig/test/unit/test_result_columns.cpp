// test/unit/test_result_columns.cpp - 算法层：结果列定义与列选择
#include <gtest/gtest.h>

#include "algorithm/result_columns.h"

using namespace psig::algorithm;

TEST(ResultColumnDefs, Count) {
    auto defs = ResultColumnDefs();
    ASSERT_EQ(defs.size(), 9u);
    EXPECT_EQ(defs[0].first, "pattern_name");
    EXPECT_EQ(defs[8].first, "custom");
    EXPECT_EQ(defs[8].second, "自定义");
}

TEST(SelectResultKeys, EmptyMeansAll) {
    auto keys = SelectResultKeys({});
    EXPECT_EQ(keys.size(), 9u);
}

TEST(SelectResultKeys, SubsetPreservesRequestedOrder) {
    auto keys = SelectResultKeys({"custom", "pid"});
    ASSERT_EQ(keys.size(), 2u);
    EXPECT_EQ(keys[0], "custom");
    EXPECT_EQ(keys[1], "pid");
}

TEST(SelectResultKeys, UnknownIgnored) {
    auto keys = SelectResultKeys({"nope", "pid"});
    ASSERT_EQ(keys.size(), 1u);
    EXPECT_EQ(keys[0], "pid");
}

TEST(SelectResultKeys, AllUnknownMeansAll) {
    auto keys = SelectResultKeys({"nope"});
    EXPECT_EQ(keys.size(), 9u);
}

TEST(FilterResultColumns, FiltersHeadersAndRows) {
    std::vector<std::string> headers = {"特征码名", "进程", "PID", "地址", "模块", "模块基址", "匹配字节", "命中时间", "自定义"};
    std::vector<std::vector<std::string>> rows = {{"p", "a.exe", "1", "0x10", "m", "0x0", "AA", "t", "c"}};
    FilterResultColumns({"pid", "custom"}, headers, rows);
    ASSERT_EQ(headers.size(), 2u);
    EXPECT_EQ(headers[0], "PID");
    EXPECT_EQ(headers[1], "自定义");
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0][0], "1");
    EXPECT_EQ(rows[0][1], "c");
}

TEST(FilterResultColumns, EmptyKeepsAll) {
    std::vector<std::string> headers = {"a", "b"};
    std::vector<std::vector<std::string>> rows = {{"1", "2"}};
    FilterResultColumns({}, headers, rows);
    EXPECT_EQ(headers.size(), 2u);
    EXPECT_EQ(rows[0].size(), 2u);
}
