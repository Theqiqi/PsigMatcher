// api/api_internal.h - 接口层内部 glue（仅 api/ 目录内共享，不外泄）
#pragma once

#include <string>

#include "psig.h"

namespace psig::api {

// 构造公共列定义（key/title/type/builtin）
inline ColumnDef MakeColumn(const std::string& key, const std::string& title,
                            const std::string& type, bool builtin) {
    ColumnDef c;
    c.key = key;
    c.title = title;
    c.type = type;
    c.builtin = builtin;
    return c;
}

} // namespace psig::api
