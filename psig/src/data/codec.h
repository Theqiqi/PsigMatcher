// data/codec.h - 纯编解码（数据层）
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace psig::data {

// 字节序列 -> 大写 hex 串，空格分隔（如 "48 8B ?? 05" 的已匹配部分输出 "48 8B 05"）
std::string HexEncode(const std::vector<uint8_t>& bytes);

// hex 串（可含空格）-> 字节序列；非法返回 false
bool HexDecode(const std::string& text, std::vector<uint8_t>& out);

// 当前时间 ISO8601（本地时间，形如 2026-10-11T12:34:56）
std::string NowIso8601();

// 宽窄字符串转换（UTF-8 <-> UTF-16）
std::string WideToUtf8(const std::wstring& w);
std::wstring Utf8ToWide(const std::string& s);

} // namespace psig::data
