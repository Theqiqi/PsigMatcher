// algorithm/result_columns.cpp - 纯算法实现（算法层，无 I/O）
#include "algorithm/result_columns.h"

namespace psig::algorithm {

std::vector<std::pair<std::string, std::string>> ResultColumnDefs() {
    return {
        {"pattern_name", "特征码名"},
        {"process_name", "进程"},
        {"pid", "PID"},
        {"address", "地址"},
        {"module_name", "模块"},
        {"module_base", "模块基址"},
        {"matched_bytes", "匹配字节"},
        {"hit_time", "命中时间"},
        {"custom", "自定义"},
    };
}

std::vector<std::string> SelectResultKeys(const std::vector<std::string>& requested) {
    const auto defs = ResultColumnDefs();
    std::vector<std::string> all;
    all.reserve(defs.size());
    for (const auto& d : defs) all.push_back(d.first);
    if (requested.empty()) return all;

    std::vector<std::string> picked;
    for (const auto& k : requested) {
        for (const auto& d : all) {
            if (d == k) {
                picked.push_back(k);
                break;
            }
        }
    }
    return picked.empty() ? all : picked;
}

void FilterResultColumns(const std::vector<std::string>& keys,
                         std::vector<std::string>& headers,
                         std::vector<std::vector<std::string>>& rows) {
    if (keys.empty()) return; // 全部列
    const auto defs = ResultColumnDefs();
    std::vector<size_t> idx;
    for (const auto& k : keys) {
        for (size_t i = 0; i < defs.size(); ++i) {
            if (defs[i].first == k) {
                idx.push_back(i);
                break;
            }
        }
    }
    if (idx.empty()) return; // 全部未知 → 视为全部

    std::vector<std::string> newHeaders;
    newHeaders.reserve(idx.size());
    for (size_t i : idx) newHeaders.push_back(defs[i].second);
    headers = newHeaders;

    for (auto& row : rows) {
        std::vector<std::string> newRow;
        newRow.reserve(idx.size());
        for (size_t i : idx) newRow.push_back(i < row.size() ? row[i] : std::string());
        row = newRow;
    }
}

} // namespace psig::algorithm
