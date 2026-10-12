// api/exchange_results.cpp - 接口层：匹配结果导出（CSV / SQLite / TXT）
#include "psig.h"

#include <sstream>

#include "algorithm/csv.h"
#include "algorithm/result_columns.h"
#include "atomic/file_io.h"
#include "atomic/sqlite_io.h"
#include "atomic/win_encoding.h"
#include "data/codec.h"

namespace psig {

namespace {

// 单个结果列键 -> 显示值
std::string ResultValueForKey(const MatchResult& m, const std::string& key) {
    if (key == "pattern_name") return m.patternName;
    if (key == "process_name") return atomic::WideToUtf8(m.processName);
    if (key == "pid") return std::to_string(m.pid);
    if (key == "address") {
        std::ostringstream o;
        o << "0x" << std::hex << std::uppercase << m.address;
        return o.str();
    }
    if (key == "address_dec") return std::to_string(m.address);
    if (key == "module_name") return atomic::WideToUtf8(m.moduleName);
    if (key == "module_base") {
        std::ostringstream o;
        o << "0x" << std::hex << std::uppercase << m.moduleBase;
        return o.str();
    }
    if (key == "matched_bytes") return m.matchedBytes;
    if (key == "hit_time") return m.hitTime;
    if (key == "custom") return m.custom;
    return std::string();
}

// 一行结果的所有值，顺序与 ResultColumnDefs() 一致（含 address_dec）
std::vector<std::string> ResultRowValues(const MatchResult& m) {
    const auto defs = algorithm::ResultColumnDefs();
    std::vector<std::string> vals;
    vals.reserve(defs.size());
    for (const auto& d : defs) vals.push_back(ResultValueForKey(m, d.first));
    return vals;
}

bool IsNumericResultKey(const std::string& k) {
    return k == "pid" || k == "address" || k == "module_base";
}

// 按 key 在列定义中查找默认中文标题；找不到返回空
std::string DefaultTitleForKey(const std::vector<std::pair<std::string, std::string>>& defs,
                               const std::string& key) {
    for (const auto& d : defs) {
        if (d.first == key) return d.second;
    }
    return std::string();
}

// key 在列定义中的下标；找不到返回 defs.size()
size_t KeyIndex(const std::vector<std::pair<std::string, std::string>>& defs,
                const std::string& key) {
    for (size_t i = 0; i < defs.size(); ++i) {
        if (defs[i].first == key) return i;
    }
    return defs.size();
}

// 按 keys 顺序构造表头：options.headers 优先，否则回落默认中文标题
std::vector<std::string> BuildResultHeaders(const std::vector<std::string>& keys,
                                            const ResultExportOptions& options) {
    const auto defs = algorithm::ResultColumnDefs();
    std::vector<std::string> headers;
    headers.reserve(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        if (!options.headers.empty() && i < options.headers.size()) {
            headers.push_back(options.headers[i]);
        } else {
            headers.push_back(DefaultTitleForKey(defs, keys[i]));
        }
    }
    return headers;
}

} // namespace

bool ExportResultsCsv(const std::string& path, const std::vector<MatchResult>& results,
                      std::string& err, const ResultExportOptions& options) {
    err.clear();
    const std::vector<std::string> keys = algorithm::SelectResultKeys(options.columns);
    const std::vector<std::string> headers = BuildResultHeaders(keys, options);
    std::vector<std::vector<std::string>> rows;
    rows.reserve(results.size());
    for (const auto& m : results) {
        std::vector<std::string> row;
        row.reserve(keys.size());
        for (const auto& k : keys) row.push_back(ResultValueForKey(m, k));
        rows.push_back(std::move(row));
    }
    std::string text = algorithm::Utf8Bom() +
                       algorithm::EncodeTemplateRecord(options.templateText) +
                       algorithm::SerializeCsv(headers, rows);
    if (!atomic::WriteTextFile(path, text)) {
        err = "无法写入文件: " + path;
        return false;
    }
    return true;
}

bool ExportResultsDb(const std::string& path, const std::vector<MatchResult>& results,
                     std::string& err, const ResultExportOptions& options) {
    err.clear();
    atomic::SqliteDb* db = atomic::SqliteOpen(path, err);
    if (!db) return false;

    const std::vector<std::string> keys = algorithm::SelectResultKeys(options.columns);
    std::string createSql = "CREATE TABLE results(";
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i) createSql += ",";
        createSql += keys[i] + (IsNumericResultKey(keys[i]) ? " INTEGER" : " TEXT");
    }
    createSql += ");";

    std::string insertSql = "INSERT INTO results(";
    std::string placeholders;
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i) { insertSql += ","; placeholders += ","; }
        insertSql += keys[i];
        placeholders += "?";
    }
    insertSql += ") VALUES(" + placeholders + ");";

    bool ok = atomic::SqliteExec(db, "DROP TABLE IF EXISTS results;", err) &&
              atomic::SqliteExec(db, "DROP TABLE IF EXISTS result_meta;", err) &&
              atomic::SqliteExec(db, createSql, err) &&
              atomic::SqliteExec(db, "CREATE TABLE result_meta(key TEXT PRIMARY KEY, value TEXT);", err);
    if (ok && !options.templateText.empty()) {
        ok = atomic::SqliteRun(db, "INSERT INTO result_meta(key,value) VALUES('template',?);",
                               {options.templateText}, err);
    }

    const auto defs = algorithm::ResultColumnDefs();
    for (const auto& m : results) {
        if (!ok) break;
        const std::vector<std::string> all = ResultRowValues(m);
        std::vector<std::string> vals;
        vals.reserve(keys.size());
        for (const auto& k : keys) {
            const size_t idx = KeyIndex(defs, k);
            vals.push_back(idx < all.size() ? all[idx] : std::string());
        }
        ok = atomic::SqliteRun(db, insertSql, vals, err);
    }
    atomic::SqliteClose(db);
    return ok;
}

bool ExportResultsTxt(const std::string& path, const std::vector<MatchResult>& results,
                      std::string& err, const ResultExportOptions& options) {
    err.clear();
    const std::vector<std::string> keys = algorithm::SelectResultKeys(options.columns);
    const std::vector<std::string> headers = BuildResultHeaders(keys, options);
    std::vector<std::vector<std::string>> rows;
    rows.reserve(results.size());
    for (const auto& m : results) {
        std::vector<std::string> row;
        row.reserve(keys.size());
        for (const auto& k : keys) row.push_back(ResultValueForKey(m, k));
        rows.push_back(std::move(row));
    }

    std::ostringstream out;
    if (!options.templateText.empty()) {
        out << options.templateText << "\n";
        out << "========================================\n";
    }
    if (!options.summary.empty()) {
        out << options.summary << "\n";
    } else {
        out << "匹配结果 (" << rows.size() << " 条)\n";
    }
    out << "========================================\n";
    for (size_t i = 0; i < rows.size(); ++i) {
        out << "[" << (i + 1) << "] ";
        for (size_t j = 0; j < headers.size(); ++j) {
            if (j) out << " | ";
            out << headers[j] << "=" << (j < rows[i].size() ? rows[i][j] : std::string());
        }
        out << "\n";
    }
    if (!atomic::WriteTextFile(path, out.str())) {
        err = "无法写入文件: " + path;
        return false;
    }
    return true;
}

} // namespace psig
