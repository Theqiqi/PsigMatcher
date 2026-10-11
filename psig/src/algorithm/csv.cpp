// algorithm/csv.cpp - 纯算法实现（算法层，无 I/O）
#include "algorithm/csv.h"

namespace psig::algorithm {

namespace {

// 将一行按逗号切分，支持双引号包裹与 "" 转义
std::vector<std::string> SplitLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    cur.push_back('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                cur.push_back(c);
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                fields.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
    }
    fields.push_back(cur);
    return fields;
}

// 转义单个字段
std::string EscapeField(const std::string& f) {
    bool need = false;
    for (char c : f) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') { need = true; break; }
    }
    if (!need) return f;
    std::string out = "\"";
    for (char c : f) {
        if (c == '"') out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

} // namespace

CsvTable ParseCsv(const std::string& text) {
    CsvTable table;
    std::vector<std::string> lines;
    std::string cur;
    bool inQuotes = false;
    for (char c : text) {
        if (c == '"') inQuotes = !inQuotes;
        if (c == '\n' && !inQuotes) {
            if (!cur.empty() && cur.back() == '\r') cur.pop_back();
            lines.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) {
        if (cur.back() == '\r') cur.pop_back();
        lines.push_back(cur);
    }
    if (lines.empty()) return table;
    table.headers = SplitLine(lines[0]);
    for (size_t i = 1; i < lines.size(); ++i) {
        if (lines[i].empty()) continue;
        table.rows.push_back(SplitLine(lines[i]));
    }
    return table;
}

std::string SerializeCsv(const std::vector<std::string>& headers,
                         const std::vector<std::vector<std::string>>& rows) {
    std::string out;
    for (size_t i = 0; i < headers.size(); ++i) {
        if (i) out.push_back(',');
        out += EscapeField(headers[i]);
    }
    out += "\r\n";
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i) out.push_back(',');
            out += EscapeField(row[i]);
        }
        out += "\r\n";
    }
    return out;
}

} // namespace psig::algorithm
