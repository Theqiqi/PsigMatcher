// algorithm/pattern.cpp - 纯算法实现（算法层，无 I/O）
#include "algorithm/pattern.h"

namespace psig::algorithm {

namespace {
int HexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
bool IsSeparator(char c) {
    return c == ' ' || c == '\t' || c == ',' || c == '\r' || c == '\n';
}
} // namespace

bool TokenToByteMask(const std::string& token, uint8_t& byte, uint8_t& mask) {
    if (token.size() != 2) return false;
    byte = 0;
    mask = 0;
    for (int i = 0; i < 2; ++i) {
        char c = token[static_cast<size_t>(i)];
        uint8_t shift = (i == 0) ? 4 : 0;
        if (c == '?') continue; // 通配：字节与掩码位均保持 0
        int v = HexVal(c);
        if (v < 0) return false;
        byte |= static_cast<uint8_t>(v << shift);
        mask |= static_cast<uint8_t>(0x0F << shift);
    }
    return true;
}

CompiledPattern CompilePattern(const std::string& raw) {
    CompiledPattern out;
    std::string compact;
    compact.reserve(raw.size());
    for (char c : raw) {
        if (IsSeparator(c)) continue;
        compact.push_back(c);
    }
    if (compact.empty()) {
        out.error = "空模式串";
        return out;
    }
    if (compact.size() % 2 != 0) {
        out.error = "模式串长度不是偶数";
        return out;
    }
    for (size_t i = 0; i < compact.size(); i += 2) {
        std::string token = compact.substr(i, 2);
        uint8_t b = 0, m = 0;
        if (!TokenToByteMask(token, b, m)) {
            out.error = "非法 token: " + token;
            return out;
        }
        out.bytes.push_back(b);
        out.mask.push_back(m);
    }
    out.valid = true;
    return out;
}

std::vector<size_t> FindAll(const uint8_t* data, size_t size,
                            const std::vector<uint8_t>& bytes,
                            const std::vector<uint8_t>& mask,
                            size_t limit) {
    std::vector<size_t> offsets;
    const size_t m = mask.size();
    if (m == 0 || size < m) return offsets;
    for (size_t i = 0; i + m <= size; ++i) {
        bool ok = true;
        for (size_t j = 0; j < m; ++j) {
            if ((data[i + j] & mask[j]) != (bytes[j] & mask[j])) {
                ok = false;
                break;
            }
        }
        if (ok) {
            offsets.push_back(i);
            if (limit != 0 && offsets.size() >= limit) break;
        }
    }
    return offsets;
}

} // namespace psig::algorithm
