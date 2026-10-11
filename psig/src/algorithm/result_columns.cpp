// algorithm/result_columns.cpp - 纯算法实现（算法层，无 I/O）
#include "algorithm/result_columns.h"

namespace psig::algorithm {

std::vector<std::pair<std::string, std::string>> ResultColumnDefs() {
    return {
        {"pattern_name", "特征码名"},
        {"process_name", "进程"},
        {"pid", "PID"},
        {"address", "地址"},
        {"address_dec", "地址(十进制)"},
        {"module_name", "模块"},
        {"module_base", "模块基址"},
        {"matched_bytes", "匹配字节"},
        {"hit_time", "命中时间"},
        {"custom", "备注"},
    };
}

std::vector<std::string> SelectResultKeys(const std::vector<std::string>& requested) {
    const auto defs = ResultColumnDefs();
    std::vector<std::string> all;
    all.reserve(defs.size());
    for (const auto& d : defs) all.push_back(d.first);

    // 默认全列不含可选十进制地址列（需显式选择）；空请求与全未知回退均用此集合
    std::vector<std::string> allDefault;
    allDefault.reserve(all.size());
    for (const auto& k : all) if (k != "address_dec") allDefault.push_back(k);

    if (requested.empty()) return allDefault;

    std::vector<std::string> picked;
    for (const auto& k : requested) {
        for (const auto& d : all) {
            if (d == k) {
                picked.push_back(k);
                break;
            }
        }
    }
    return picked.empty() ? allDefault : picked;
}

} // namespace psig::algorithm
