// test/unit/test_csv.cpp - 算法层：CSV 解析与序列化
#include <gtest/gtest.h>

#include "algorithm/csv.h"

using namespace psig::algorithm;

TEST(ParseCsv, Simple) {
    auto t = ParseCsv("a,b,c\r\n1,2,3\r\n4,5,6\r\n");
    ASSERT_EQ(t.headers.size(), 3u);
    EXPECT_EQ(t.headers[0], "a");
    ASSERT_EQ(t.rows.size(), 2u);
    EXPECT_EQ(t.rows[1][2], "6");
}

TEST(ParseCsv, QuotedFields) {
    auto t = ParseCsv("name,note\r\n\"hello, world\",\"say \"\"hi\"\"\"\r\n");
    ASSERT_EQ(t.rows.size(), 1u);
    EXPECT_EQ(t.rows[0][0], "hello, world");
    EXPECT_EQ(t.rows[0][1], "say \"hi\"");
}

TEST(ParseCsv, LfOnly) {
    auto t = ParseCsv("x,y\n1,2\n");
    ASSERT_EQ(t.rows.size(), 1u);
    EXPECT_EQ(t.rows[0][0], "1");
}

TEST(SerializeCsv, RoundTrip) {
    std::vector<std::string> headers = {"名称", "特征码", "备注"};
    std::vector<std::vector<std::string>> rows = {
        {"p1", "48 8B ?? 05", "含,逗号"},
        {"p2", "DE AD BE EF", "含\"引号\""},
    };
    std::string text = SerializeCsv(headers, rows);
    auto t = ParseCsv(text);
    ASSERT_EQ(t.headers.size(), 3u);
    ASSERT_EQ(t.rows.size(), 2u);
    EXPECT_EQ(t.rows[0][2], "含,逗号");
    EXPECT_EQ(t.rows[1][2], "含\"引号\"");
}

TEST(SerializeCsv, EmptyRows) {
    std::string text = SerializeCsv({"a", "b"}, {});
    EXPECT_NE(text.find("a,b"), std::string::npos);
}
