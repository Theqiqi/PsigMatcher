// algorithm/result_columns.h - 纯算法：匹配结果列定义与列选择（算法层，无 I/O）
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace psig::algorithm {

// 结果列定义：(键, 标题)，顺序即导出顺序
std::vector<std::pair<std::string, std::string>> ResultColumnDefs();

// 选择结果列键：requested 为空 → 全部键；含未知键 → 忽略；全部未知 → 全部键
std::vector<std::string> SelectResultKeys(const std::vector<std::string>& requested);

// 按列键过滤表头与行（keys 为空 → 全部；含未知键 → 忽略未知键；全部未知 → 全部）
void FilterResultColumns(const std::vector<std::string>& keys,
                         std::vector<std::string>& headers,
                         std::vector<std::vector<std::string>>& rows);

} // namespace psig::algorithm
