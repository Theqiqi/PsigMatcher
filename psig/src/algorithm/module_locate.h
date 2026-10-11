// algorithm/module_locate.h - 纯算法：地址归属模块（算法层）
#pragma once

#include <cstdint>
#include <vector>

#include "data/types.h"

namespace psig::algorithm {

// 返回包含 addr 的模块下标；无则返回 -1
int LocateModule(uint64_t addr, const std::vector<data::ModuleInfo>& modules);

} // namespace psig::algorithm
