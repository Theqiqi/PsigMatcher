// test/unit/test_result_columns.cpp - 算法层：结果列定义与列选择
#include <gtest/gtest.h>

#include "algorithm/result_columns.h"

using namespace psig::algorithm;

TEST(ResultColumnDefs, Count) {
    auto defs = ResultColumnDefs();
    ASSERT_EQ(defs.size(), 10u);
    EXPECT_EQ(defs[0].first, "pattern_name");
    EXPECT_EQ(defs[3].first, "address");
    EXPECT_EQ(defs[4].first, "address_dec");
    EXPECT_EQ(defs[4].second, "地址(十进制)");
    EXPECT_EQ(defs[9].first, "custom");
    EXPECT_EQ(defs[9].second, "备注");
}

TEST(ResultColumnDefs, DecimalAddressExcludedFromDefaultExport) {
    // SelectResultKeys 默认全列不应包含 address_dec（需显式选择）
    auto keys = SelectResultKeys({});
    for (const auto& k : keys) EXPECT_NE(k, "address_dec");
    auto explicitKeys = SelectResultKeys({"address_dec"});
    ASSERT_EQ(explicitKeys.size(), 1u);
    EXPECT_EQ(explicitKeys[0], "address_dec");
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
