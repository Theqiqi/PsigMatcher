// api/psig.cpp - 接口层：组装 原子化底层 + 算法层 + 数据层，实现公共 API
#include "psig.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "algorithm/csv.h"
#include "algorithm/module_locate.h"
#include "algorithm/pattern.h"
#include "algorithm/result_columns.h"
#include "atomic/file_io.h"
#include "atomic/sqlite_io.h"
#include "atomic/win_icon.h"
#include "atomic/win_process.h"
#include "data/codec.h"

namespace psig {

namespace {

constexpr size_t kChunkSize = 1u << 20; // 1 MiB 分块读取

std::atomic_bool g_cancel{false};

bool IsCancelled(const std::atomic_bool* cancel) {
    return g_cancel.load() || (cancel && cancel->load());
}

// ---- 动态列表格的 cells 编解码（长度前缀，形如 5:hello3:abc） ----
std::string EncodeCells(const std::vector<std::string>& cells) {
    std::string out;
    for (const auto& c : cells) {
        out += std::to_string(c.size());
        out.push_back(':');
        out += c;
    }
    return out;
}

bool DecodeCells(const std::string& text, std::vector<std::string>& cells) {
    cells.clear();
    size_t i = 0;
    while (i < text.size()) {
        size_t colon = text.find(':', i);
        if (colon == std::string::npos) return false;
        size_t len = 0;
        for (size_t k = i; k < colon; ++k) {
            if (text[k] < '0' || text[k] > '9') return false;
            len = len * 10 + static_cast<size_t>(text[k] - '0');
        }
        size_t start = colon + 1;
        if (start + len > text.size()) return false;
        cells.push_back(text.substr(start, len));
        i = start + len;
    }
    return true;
}

ColumnDef MakeColumn(const std::string& key, const std::string& title,
                     const std::string& type, bool builtin) {
    ColumnDef c;
    c.key = key;
    c.title = title;
    c.type = type;
    c.builtin = builtin;
    return c;
}

// CSV 表头标题 -> 内置列键；非内置返回空（支持中英文标题，保证跨语言导入往返）
std::string BuiltinKeyFromTitle(const std::string& title) {
    if (title == "name" || title == "名称" || title == "Name") return "name";
    if (title == "pattern" || title == "特征码" || title == "Signature" || title == "Pattern") return "pattern";
    if (title == "note" || title == "备注" || title == "Note" || title == "Remarks") return "note";
    return {};
}

std::string SanitizeKey(const std::string& title, size_t index) {
    std::string key;
    for (char c : title) {
        if (std::isalnum(static_cast<unsigned char>(c))) key.push_back(c);
        else key.push_back('_');
    }
    if (key.empty()) key = "col";
    return key + "_" + std::to_string(index);
}

} // namespace

// ---------------------------------------------------------------------------
// 进程 / 扫描
// ---------------------------------------------------------------------------

std::vector<ProcessInfo> EnumerateProcesses() {
    std::vector<ProcessInfo> result;
    for (uint32_t pid : atomic::EnumProcessIds()) {
        ProcessInfo info;
        info.pid = pid;
        std::wstring name, path;
        atomic::ProcessNamePath(pid, name, path);
        info.name = name;
        info.path = path;
        void* h = atomic::OpenForRead(pid);
        info.accessible = (h != nullptr);
        atomic::CloseProcessHandle(h);
        result.push_back(std::move(info));
    }
    std::sort(result.begin(), result.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) { return a.pid < b.pid; });
    return result;
}

namespace {
IconBitmap ToPublicBitmap(atomic::RawBitmap&& raw) {
    IconBitmap out;
    out.width = raw.width;
    out.height = raw.height;
    out.pixels = std::move(raw.pixels);
    return out;
}
} // namespace

IconBitmap ProcessIcon(uint32_t pid) {
    return ToPublicBitmap(atomic::ExtractProcessIcon(pid));
}

IconBitmap DefaultProcessIcon() {
    return ToPublicBitmap(atomic::ExtractDefaultIcon());
}

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

std::vector<MatchResult> ScanProcesses(const ScanRequest& req,
                                       ProgressCallback progress,
                                       const std::atomic_bool* cancel) {
    std::vector<MatchResult> results;
    // 过滤合法模式
    std::vector<Pattern> patterns;
    for (const auto& p : req.patterns) {
        if (p.valid && !p.bytes.empty()) patterns.push_back(p);
    }
    const size_t total = patterns.size();
    const size_t maxLen = [&]() {
        size_t m = 0;
        for (const auto& p : patterns) m = (std::max)(m, p.bytes.size());
        return m;
    }();

    for (uint32_t pid : req.pids) {
        if (IsCancelled(cancel)) break;
        void* h = atomic::OpenForRead(pid);
        if (!h) continue;
        std::wstring pname, ppath;
        atomic::ProcessNamePath(pid, pname, ppath);
        std::vector<data::ModuleInfo> modules = atomic::EnumModules(pid);
        std::vector<data::Region> regions = atomic::EnumRegions(h);

        for (const auto& region : regions) {
            if (IsCancelled(cancel)) break;
            uint64_t offset = 0;
            std::vector<uint8_t> carry;
            while (offset < region.size) {
                if (IsCancelled(cancel)) break;
                size_t want = static_cast<size_t>(
                    (std::min)(static_cast<uint64_t>(kChunkSize), region.size - offset));
                std::vector<uint8_t> chunk = atomic::ReadMemory(h, region.base + offset, want);
                if (chunk.empty()) {
                    offset += want;
                    carry.clear();
                    continue;
                }
                // 拼接上一块尾部，保证跨块模式可命中
                std::vector<uint8_t> buffer;
                buffer.reserve(carry.size() + chunk.size());
                buffer.insert(buffer.end(), carry.begin(), carry.end());
                buffer.insert(buffer.end(), chunk.begin(), chunk.end());
                const uint64_t bufferBase = region.base + offset - carry.size();

                for (const auto& p : patterns) {
                    if (IsCancelled(cancel)) break;
                    size_t limit = req.maxResultsPerPattern;
                    std::vector<size_t> hits =
                        algorithm::FindAll(buffer.data(), buffer.size(), p.bytes, p.mask, limit);
                    for (size_t pos : hits) {
                        MatchResult mr;
                        mr.patternName = p.name;
                        mr.pid = pid;
                        mr.processName = pname;
                        mr.address = bufferBase + pos;
                        int modIdx = algorithm::LocateModule(mr.address, modules);
                        if (modIdx >= 0) {
                            mr.moduleName = modules[modIdx].name;
                            mr.moduleBase = modules[modIdx].base;
                        }
                        std::vector<uint8_t> matched(buffer.begin() + static_cast<long>(pos),
                                                     buffer.begin() + static_cast<long>(pos + p.bytes.size()));
                        mr.matchedBytes = data::HexEncode(matched);
                        mr.hitTime = data::NowIso8601();
                        results.push_back(std::move(mr));
                    }
                }
                // 保留尾部 overlap
                size_t keep = (maxLen > 1) ? (maxLen - 1) : 0;
                if (keep > 0 && buffer.size() > keep) {
                    carry.assign(buffer.end() - static_cast<long>(keep), buffer.end());
                } else {
                    carry = buffer;
                }
                offset += want;
                if (progress) {
                    ScanProgress sp;
                    sp.currentPid = pid;
                    sp.done = total;
                    sp.total = total;
                    sp.hits = results.size();
                    progress(sp);
                }
            }
        }
        atomic::CloseProcessHandle(h);
    }
    return results;
}

void RequestCancel(bool flag) { g_cancel.store(flag); }

std::vector<ColumnDef> BuiltinSignatureColumns() {
    return {
        MakeColumn("name", "名称", "text", true),
        MakeColumn("pattern", "特征码", "text", true),
        MakeColumn("note", "备注", "text", true),
    };
}

// ---------------------------------------------------------------------------
// 数据交换：特征码表
// ---------------------------------------------------------------------------

Table ImportSignaturesCsv(const std::string& path, std::string& err) {
    Table table;
    err.clear();
    std::string text;
    if (!atomic::ReadTextFile(path, text)) {
        err = "无法读取文件: " + path;
        return table;
    }
    // 剥离文件头 UTF-8 BOM（若有），避免污染首个表头/模板记录
    text = algorithm::StripUtf8Bom(text);
    // 前置模板记录（若存在）
    std::string tmpl, rest;
    if (algorithm::ParseTemplateRecord(text, tmpl, rest)) {
        table.templateText = tmpl;
    } else {
        rest = text;
    }
    algorithm::CsvTable csv = algorithm::ParseCsv(rest);
    for (size_t i = 0; i < csv.headers.size(); ++i) {
        const std::string& title = csv.headers[i];
        std::string builtinKey = BuiltinKeyFromTitle(title);
        if (!builtinKey.empty()) {
            table.columns.push_back(MakeColumn(builtinKey, title, "text", true));
        } else {
            table.columns.push_back(MakeColumn(SanitizeKey(title, i), title, "text", false));
        }
    }
    for (const auto& row : csv.rows) {
        TableRow tr;
        tr.cells = row;
        table.rows.push_back(std::move(tr));
    }
    return table;
}

Table ImportSignaturesDb(const std::string& path, std::string& err) {
    Table table;
    err.clear();
    atomic::SqliteDb* db = atomic::SqliteOpen(path, err);
    if (!db) return table;
    if (!atomic::SqliteTableExists(db, "sig_columns")) {
        err = "数据库缺少 sig_columns 表（非本程序导出的特征码库）";
        atomic::SqliteClose(db);
        return table;
    }
    std::vector<std::vector<std::string>> cols;
    if (!atomic::SqliteQuery(db,
            "SELECT key,title,type,builtin FROM sig_columns ORDER BY ord;", cols, err)) {
        atomic::SqliteClose(db);
        return table;
    }
    for (const auto& c : cols) {
        bool builtin = c.size() > 3 && c[3] == "1";
        table.columns.push_back(MakeColumn(c[0], c[1], c.size() > 2 ? c[2] : "text", builtin));
    }
    std::vector<std::vector<std::string>> rows;
    if (!atomic::SqliteQuery(db, "SELECT cells FROM sig_rows ORDER BY id;", rows, err)) {
        atomic::SqliteClose(db);
        return table;
    }
    for (const auto& r : rows) {
        TableRow tr;
        if (!r.empty()) DecodeCells(r[0], tr.cells);
        table.rows.push_back(std::move(tr));
    }
    // 表级模板文本（旧库无 sig_meta 时忽略）
    if (atomic::SqliteTableExists(db, "sig_meta")) {
        std::vector<std::vector<std::string>> meta;
        std::string metaErr;
        if (atomic::SqliteQuery(db, "SELECT value FROM sig_meta WHERE key='template';", meta, metaErr) &&
            !meta.empty() && !meta[0].empty()) {
            table.templateText = meta[0][0];
        }
    }
    atomic::SqliteClose(db);
    return table;
}

bool ExportSignaturesCsv(const std::string& path, const Table& table, std::string& err) {
    err.clear();
    std::vector<std::string> headers;
    for (const auto& c : table.columns) headers.push_back(c.title);
    std::vector<std::vector<std::string>> rows;
    for (const auto& r : table.rows) rows.push_back(r.cells);
    std::string text = algorithm::Utf8Bom() +
                       algorithm::EncodeTemplateRecord(table.templateText) +
                       algorithm::SerializeCsv(headers, rows);
    if (!atomic::WriteTextFile(path, text)) {
        err = "无法写入文件: " + path;
        return false;
    }
    return true;
}

bool ExportSignaturesDb(const std::string& path, const Table& table, std::string& err) {
    err.clear();
    atomic::SqliteDb* db = atomic::SqliteOpen(path, err);
    if (!db) return false;
    bool ok = atomic::SqliteExec(db, "DROP TABLE IF EXISTS sig_columns;", err) &&
              atomic::SqliteExec(db, "DROP TABLE IF EXISTS sig_rows;", err) &&
              atomic::SqliteExec(db, "DROP TABLE IF EXISTS sig_meta;", err) &&
              atomic::SqliteExec(db,
                  "CREATE TABLE sig_columns(key TEXT PRIMARY KEY, title TEXT, type TEXT, "
                  "builtin INTEGER, ord INTEGER);", err) &&
              atomic::SqliteExec(db,
                  "CREATE TABLE sig_rows(id INTEGER PRIMARY KEY AUTOINCREMENT, cells TEXT);", err) &&
              atomic::SqliteExec(db,
                  "CREATE TABLE sig_meta(key TEXT PRIMARY KEY, value TEXT);", err);
    if (ok && !table.templateText.empty()) {
        ok = atomic::SqliteRun(db, "INSERT INTO sig_meta(key,value) VALUES('template',?);",
                               {table.templateText}, err);
    }
    if (ok) {
        for (size_t i = 0; i < table.columns.size() && ok; ++i) {
            const auto& c = table.columns[i];
            ok = atomic::SqliteRun(db,
                "INSERT INTO sig_columns(key,title,type,builtin,ord) VALUES(?,?,?,?,?);",
                {c.key, c.title, c.type, c.builtin ? "1" : "0", std::to_string(i)}, err);
        }
    }
    if (ok) {
        for (const auto& r : table.rows) {
            ok = atomic::SqliteRun(db, "INSERT INTO sig_rows(cells) VALUES(?);",
                                   {EncodeCells(r.cells)}, err);
            if (!ok) break;
        }
    }
    atomic::SqliteClose(db);
    return ok;
}

// ---------------------------------------------------------------------------
// 数据交换：匹配结果
// ---------------------------------------------------------------------------

namespace {
// 单个结果列键 -> 显示值
std::string ResultValueForKey(const MatchResult& m, const std::string& key) {
    if (key == "pattern_name") return m.patternName;
    if (key == "process_name") return data::WideToUtf8(m.processName);
    if (key == "pid") return std::to_string(m.pid);
    if (key == "address") {
        std::ostringstream o;
        o << "0x" << std::hex << std::uppercase << m.address;
        return o.str();
    }
    if (key == "address_dec") return std::to_string(m.address);
    if (key == "module_name") return data::WideToUtf8(m.moduleName);
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
} // namespace

bool ExportResultsCsv(const std::string& path, const std::vector<MatchResult>& results,
                      std::string& err, const ResultExportOptions& options) {
    err.clear();
    const auto defs = algorithm::ResultColumnDefs();
    const std::vector<std::string> keys = algorithm::SelectResultKeys(options.columns);
    std::vector<std::string> headers;
    headers.reserve(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string h;
        if (!options.headers.empty() && i < options.headers.size()) h = options.headers[i];
        else for (const auto& d : defs) if (d.first == keys[i]) { h = d.second; break; }
        headers.push_back(h);
    }
    std::vector<std::vector<std::string>> rows;
    rows.reserve(results.size());
    for (const auto& m : results) {
        std::vector<std::string> row; row.reserve(keys.size());
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
            size_t idx = 0;
            for (size_t i = 0; i < defs.size(); ++i) {
                if (defs[i].first == k) { idx = i; break; }
            }
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
    const auto defs = algorithm::ResultColumnDefs();
    const std::vector<std::string> keys = algorithm::SelectResultKeys(options.columns);
    std::vector<std::string> headers;
    headers.reserve(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string h;
        if (!options.headers.empty() && i < options.headers.size()) h = options.headers[i];
        else for (const auto& d : defs) if (d.first == keys[i]) { h = d.second; break; }
        headers.push_back(h);
    }
    std::vector<std::vector<std::string>> rows;
    rows.reserve(results.size());
    for (const auto& m : results) {
        std::vector<std::string> row; row.reserve(keys.size());
        for (const auto& k : keys) row.push_back(ResultValueForKey(m, k));
        rows.push_back(std::move(row));
    }

    std::ostringstream out;
    if (!options.templateText.empty()) {
        out << options.templateText << "\n";
        out << "========================================\n";
    }
    out << "匹配结果 (" << rows.size() << " 条)\n";
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
