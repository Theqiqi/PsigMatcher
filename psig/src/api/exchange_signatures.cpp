// api/exchange_signatures.cpp - 接口层：特征码表导入/导出（CSV / SQLite）
#include "psig.h"

#include <cctype>

#include "algorithm/csv.h"
#include "api/api_internal.h"
#include "atomic/file_io.h"
#include "atomic/sqlite_io.h"
#include "data/codec.h"

namespace psig {

namespace {

// CSV 表头标题 -> 内置列键；非内置返回空（支持中英文标题，保证跨语言导入往返）
std::string BuiltinKeyFromTitle(const std::string& title) {
    if (title == "name" || title == "名称" || title == "Name") return "name";
    if (title == "pattern" || title == "特征码" || title == "Signature" || title == "Pattern")
        return "pattern";
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
        const std::string builtinKey = BuiltinKeyFromTitle(title);
        if (!builtinKey.empty()) {
            table.columns.push_back(api::MakeColumn(builtinKey, title, "text", true));
        } else {
            table.columns.push_back(api::MakeColumn(SanitizeKey(title, i), title, "text", false));
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
        const bool builtin = c.size() > 3 && c[3] == "1";
        table.columns.push_back(api::MakeColumn(c[0], c[1], c.size() > 2 ? c[2] : "text", builtin));
    }
    std::vector<std::vector<std::string>> rows;
    if (!atomic::SqliteQuery(db, "SELECT cells FROM sig_rows ORDER BY id;", rows, err)) {
        atomic::SqliteClose(db);
        return table;
    }
    for (const auto& r : rows) {
        TableRow tr;
        if (!r.empty()) data::DecodeCells(r[0], tr.cells);
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
                                   {data::EncodeCells(r.cells)}, err);
            if (!ok) break;
        }
    }
    atomic::SqliteClose(db);
    return ok;
}

} // namespace psig
