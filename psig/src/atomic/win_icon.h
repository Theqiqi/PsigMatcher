// atomic/win_icon.h - 原子化底层：从可执行文件提取图标为 BGRA 位图（Windows API）
#pragma once

#include <cstdint>
#include <vector>

namespace psig::atomic {

// 原始位图（BGRA，自上而下，stride = width*4）；width==0 表示失败
struct RawBitmap {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;
    bool valid() const {
        return width > 0 && height > 0 &&
               pixels.size() == static_cast<size_t>(width) * height * 4;
    }
};

// 提取指定进程可执行文件的图标；失败返回 width==0 的空位图
RawBitmap ExtractProcessIcon(uint32_t pid);

// 系统默认可执行文件图标；通常有效
RawBitmap ExtractDefaultIcon();

} // namespace psig::atomic
