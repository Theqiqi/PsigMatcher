// atomic/file_io.h - 原子化底层：文件文本 I/O
#pragma once

#include <string>

namespace psig::atomic {

// 读取整个文本文件（二进制读入）；失败返回 false
bool ReadTextFile(const std::string& path, std::string& out);

// 写入整个文本文件；失败返回 false
bool WriteTextFile(const std::string& path, const std::string& text);

} // namespace psig::atomic
