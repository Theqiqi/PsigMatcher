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

// 取首条逻辑 CSV 记录（尊重引号，含跨行引号字段）；record 去尾 \r
void TakeFirstRecord(const std::string& text, std::string& record, std::string& rest) {
    size_t i = 0;
    bool inQuotes = false;
    for (; i < text.size(); ++i) {
        char c = text[i];
        if (c == '"') {
            if (inQuotes && i + 1 < text.size() && text[i + 1] == '"') { ++i; continue; }
            inQuotes = !inQuotes;
        } else if (c == '\n' && !inQuotes) {
            break;
        }
    }
    if (i < text.size()) {
        record = text.substr(0, i);
        rest = text.substr(i + 1);
    } else {
        record = text;
        rest.clear();
    }
    if (!record.empty() && record.back() == '\r') record.pop_back();
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

std::string EncodeTemplateRecord(const std::string& tmpl) {
    if (tmpl.empty()) return {};
    return "#psig-template," + EscapeField(tmpl) + "\r\n";
}

bool ParseTemplateRecord(const std::string& text, std::string& tmpl, std::string& rest) {
    tmpl.clear();
    rest = text;
    std::string record, tail;
    TakeFirstRecord(text, record, tail);
    std::vector<std::string> fields = SplitLine(record);
    if (fields.size() >= 2 && fields[0] == "#psig-template") {
        tmpl = fields[1];
        rest = tail;
        return true;
    }
    return false;
}

} // namespace psig::algorithm
