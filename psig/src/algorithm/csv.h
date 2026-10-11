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

// 表级模板文本的前置记录（置于 CSV 文件最前）。
// 编码：返回 "#psig-template,<escaped>\r\n"；tmpl 为空时返回空串。
std::string EncodeTemplateRecord(const std::string& tmpl);

// 解析前置模板记录。
// 若 text 首条 CSV 记录首字段为 "#psig-template"，则 tmpl=第二字段、rest=其余文本、返回 true；
// 否则 tmpl 置空、rest=text、返回 false。
bool ParseTemplateRecord(const std::string& text, std::string& tmpl, std::string& rest);

// UTF-8 BOM（字节序标记）：返回 3 字节 "\xEF\xBB\xBF"。
// 用途：CSV 导出置于文件最前，使 Excel 等按 UTF-8 解析中文表头。
std::string Utf8Bom();

// 判断文本是否以 UTF-8 BOM 开头。
bool HasUtf8Bom(const std::string& text);

// 若文本以 UTF-8 BOM 开头则返回去掉 BOM 的文本，否则原样返回。
std::string StripUtf8Bom(const std::string& text);

} // namespace psig::algorithm
