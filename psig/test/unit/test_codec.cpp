// test/unit/test_codec.cpp - 数据层：编解码
#include <gtest/gtest.h>

#include "data/codec.h"

using namespace psig::data;

TEST(HexEncode, Basic) {
    std::vector<uint8_t> b = {0x00, 0x48, 0xAB, 0xFF};
    EXPECT_EQ(HexEncode(b), "00 48 AB FF");
}

TEST(HexDecode, WithSpaces) {
    std::vector<uint8_t> out;
    ASSERT_TRUE(HexDecode("48 8B 05", out));
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], 0x48);
    EXPECT_EQ(out[2], 0x05);
}

TEST(HexDecode, Invalid) {
    std::vector<uint8_t> out;
    EXPECT_FALSE(HexDecode("4Z", out));
    EXPECT_FALSE(HexDecode("4", out));
}

TEST(HexCodec, RoundTrip) {
    std::vector<uint8_t> b = {0xDE, 0xAD, 0xBE, 0xEF};
    std::vector<uint8_t> out;
    ASSERT_TRUE(HexDecode(HexEncode(b), out));
    EXPECT_EQ(b, out);
}

TEST(NowIso8601, Format) {
    std::string s = NowIso8601();
    EXPECT_EQ(s.size(), 19u);
    EXPECT_EQ(s[4], '-');
    EXPECT_EQ(s[10], 'T');
}

TEST(CellsCodec, RoundTrip) {
    std::vector<std::string> cells = {"hello", "", "abc", "中文"};
    std::vector<std::string> out;
    ASSERT_TRUE(DecodeCells(EncodeCells(cells), out));
    EXPECT_EQ(cells, out);
}

TEST(CellsCodec, EmptyList) {
    std::vector<std::string> out;
    EXPECT_TRUE(DecodeCells(EncodeCells({}), out));
    EXPECT_TRUE(out.empty());
}

TEST(CellsCodec, InvalidInput) {
    std::vector<std::string> out;
    EXPECT_FALSE(DecodeCells("5:ab", out));  // 长度超出文本
    EXPECT_FALSE(DecodeCells("x:ab", out));  // 长度非数字
    EXPECT_FALSE(DecodeCells("5abc", out));  // 缺冒号
}
