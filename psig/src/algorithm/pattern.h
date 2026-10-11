// algorithm/pattern.h - 纯算法：模式解析与匹配（算法层）
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace psig::algorithm {

// 编译后的模式（纯数据）
struct CompiledPattern {
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> mask;
    bool valid = false;
    std::string error;
};

// 单个 2 字符 token -> (期望字节, 掩码)
// 规则：'??' -> (0,0)；'XY' -> (XY,0xFF)；'?Y' -> (0Y,0x0F)；'X?' -> (X0,0xF0)
bool TokenToByteMask(const std::string& token, uint8_t& byte, uint8_t& mask);

// 解析模式串（去空格/逗号后按 2 字符切分）
CompiledPattern CompilePattern(const std::string& raw);

// 在缓冲区中查找全部匹配起点；limit==0 表示不限
std::vector<size_t> FindAll(const uint8_t* data, size_t size,
                            const std::vector<uint8_t>& bytes,
                            const std::vector<uint8_t>& mask,
                            size_t limit);

} // namespace psig::algorithm
