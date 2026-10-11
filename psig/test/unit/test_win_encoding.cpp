// test/unit/test_win_encoding.cpp - 原子化层：UTF-16 <-> UTF-8 转换
#include <gtest/gtest.h>

#include "atomic/win_encoding.h"

using namespace psig::atomic;

TEST(WideUtf8, RoundTrip) {
    std::wstring w = L"进程特征码";
    EXPECT_EQ(Utf8ToWide(WideToUtf8(w)), w);
}

TEST(WideUtf8, Empty) {
    EXPECT_EQ(WideToUtf8(L""), "");
    EXPECT_EQ(Utf8ToWide(""), std::wstring());
}

TEST(WideUtf8, Ascii) {
    EXPECT_EQ(WideToUtf8(L"abc"), "abc");
    EXPECT_EQ(Utf8ToWide("abc"), L"abc");
}
