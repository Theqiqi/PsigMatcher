// atomic/win_encoding.h - 原子化底层：UTF-16 <-> UTF-8 转换（Windows API）
#pragma once

#include <string>

namespace psig::atomic {

// 宽字符串（UTF-16）-> UTF-8；失败返回空串
std::string WideToUtf8(const std::wstring& w);

// UTF-8 -> 宽字符串（UTF-16）；失败返回空串
std::wstring Utf8ToWide(const std::string& s);

} // namespace psig::atomic
