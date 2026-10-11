// api/pattern_api.cpp - 接口层：模式解析 / 内置列定义 / 结果列查询
#include "psig.h"

#include "algorithm/pattern.h"
#include "algorithm/result_columns.h"
#include "api/api_internal.h"

namespace psig {

Pattern ParsePattern(const std::string& raw, const std::string& name) {
    Pattern p;
    p.name = name;
    p.raw = raw;
    algorithm::CompiledPattern c = algorithm::CompilePattern(raw);
    p.valid = c.valid;
    p.error = c.error;
    p.bytes = c.bytes;
    p.mask = c.mask;
    return p;
}

std::vector<ColumnDef> BuiltinSignatureColumns() {
    return {
        api::MakeColumn("name", "名称", "text", true),
        api::MakeColumn("pattern", "特征码", "text", true),
        api::MakeColumn("note", "备注", "text", true),
    };
}

std::vector<std::pair<std::string, std::string>> ResultColumnDefs() {
    return algorithm::ResultColumnDefs();
}

std::vector<std::string> SelectResultKeys(const std::vector<std::string>& requested) {
    return algorithm::SelectResultKeys(requested);
}

} // namespace psig
