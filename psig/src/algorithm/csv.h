// algorithm/csv.h - 纯算法：CSV 文本 <-> 表格（算法层，无 I/O）
#pragma once

#include <string>
#include <vector>

namespace psig::algorithm {

// 解析后的 CSV（首行为表头）
struct CsvTable {
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
};

// 解析 CSV 文本（支持引号包裹、逗号分隔、CRLF/LF）
CsvTable ParseCsv(const std::string& text);

// 序列化为 CSV 文本（含表头；必要时加引号转义）
std::string SerializeCsv(const std::vector<std::string>& headers,
                         const std::vector<std::vector<std::string>>& rows);

} // namespace psig::algorithm
