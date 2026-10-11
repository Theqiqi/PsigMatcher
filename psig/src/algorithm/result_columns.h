// algorithm/result_columns.h - 纯算法：匹配结果列定义与列选择（算法层，无 I/O）
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace psig::algorithm {

// 结果列定义：(键, 标题)，顺序即导出顺序
std::vector<std::pair<std::string, std::string>> ResultColumnDefs();

// 选择结果列键：requested 为空 → 全部键（不含 address_dec）；含未知键 → 忽略；全部未知 → 全部键
std::vector<std::string> SelectResultKeys(const std::vector<std::string>& requested);

} // namespace psig::algorithm
