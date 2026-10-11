// algorithm/module_locate.cpp - 纯算法实现（算法层）
#include "algorithm/module_locate.h"

namespace psig::algorithm {

int LocateModule(uint64_t addr, const std::vector<data::ModuleInfo>& modules) {
    for (size_t i = 0; i < modules.size(); ++i) {
        const auto& m = modules[i];
        if (addr >= m.base && addr < m.base + m.size) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace psig::algorithm
