// test/unit/test_pattern.cpp - 算法层：模式解析与匹配
#include <gtest/gtest.h>

#include "algorithm/pattern.h"

using namespace psig::algorithm;

TEST(TokenToByteMask, ExactByte) {
    uint8_t b = 0, m = 0;
    ASSERT_TRUE(TokenToByteMask("8B", b, m));
    EXPECT_EQ(b, 0x8B);
    EXPECT_EQ(m, 0xFF);
}

TEST(TokenToByteMask, FullWildcard) {
    uint8_t b = 0, m = 0;
    ASSERT_TRUE(TokenToByteMask("??", b, m));
    EXPECT_EQ(b, 0x00);
    EXPECT_EQ(m, 0x00);
}

TEST(TokenToByteMask, HighNibbleWildcard) {
    uint8_t b = 0, m = 0;
    ASSERT_TRUE(TokenToByteMask("?B", b, m));
    EXPECT_EQ(b, 0x0B);
    EXPECT_EQ(m, 0x0F);
}

TEST(TokenToByteMask, LowNibbleWildcard) {
    uint8_t b = 0, m = 0;
    ASSERT_TRUE(TokenToByteMask("8?", b, m));
    EXPECT_EQ(b, 0x80);
    EXPECT_EQ(m, 0xF0);
}

TEST(TokenToByteMask, Invalid) {
    uint8_t b = 0, m = 0;
    EXPECT_FALSE(TokenToByteMask("ZZ", b, m));
    EXPECT_FALSE(TokenToByteMask("8", b, m));
}

TEST(CompilePattern, WithSpaces) {
    auto p = CompilePattern("48 8B ?? 05");
    ASSERT_TRUE(p.valid) << p.error;
    ASSERT_EQ(p.bytes.size(), 4u);
    EXPECT_EQ(p.bytes[0], 0x48);
    EXPECT_EQ(p.mask[2], 0x00);
}

TEST(CompilePattern, Contiguous) {
    auto p = CompilePattern("488B??05");
    ASSERT_TRUE(p.valid);
    EXPECT_EQ(p.bytes.size(), 4u);
}

TEST(CompilePattern, Empty) {
    EXPECT_FALSE(CompilePattern("").valid);
}

TEST(CompilePattern, OddLength) {
    EXPECT_FALSE(CompilePattern("48 8B 0").valid);
}

TEST(FindAll, ExactSingle) {
    std::vector<uint8_t> data = {0x00, 0x48, 0x8B, 0x05, 0x00};
    std::vector<uint8_t> bytes = {0x48, 0x8B, 0x05};
    std::vector<uint8_t> mask = {0xFF, 0xFF, 0xFF};
    auto offs = FindAll(data.data(), data.size(), bytes, mask, 0);
    ASSERT_EQ(offs.size(), 1u);
    EXPECT_EQ(offs[0], 1u);
}

TEST(FindAll, Wildcard) {
    std::vector<uint8_t> data = {0x48, 0x8B, 0xAA, 0x05, 0x48, 0x8B, 0xBB, 0x05};
    auto p = CompilePattern("48 8B ?? 05");
    auto offs = FindAll(data.data(), data.size(), p.bytes, p.mask, 0);
    ASSERT_EQ(offs.size(), 2u);
    EXPECT_EQ(offs[0], 0u);
    EXPECT_EQ(offs[1], 4u);
}

TEST(FindAll, PartialNibble) {
    std::vector<uint8_t> data = {0x8B, 0x8C, 0x9B};
    auto p = CompilePattern("8?");
    auto offs = FindAll(data.data(), data.size(), p.bytes, p.mask, 0);
    ASSERT_EQ(offs.size(), 2u); // 0x8B 与 0x8C 命中，0x9B 不命中
}

TEST(FindAll, Limit) {
    std::vector<uint8_t> data = {0xAA, 0xAA, 0xAA};
    auto p = CompilePattern("AA");
    auto offs = FindAll(data.data(), data.size(), p.bytes, p.mask, 2);
    EXPECT_EQ(offs.size(), 2u);
}

TEST(FindAll, PatternLongerThanData) {
    std::vector<uint8_t> data = {0xAA};
    auto p = CompilePattern("AA BB");
    auto offs = FindAll(data.data(), data.size(), p.bytes, p.mask, 0);
    EXPECT_TRUE(offs.empty());
}
