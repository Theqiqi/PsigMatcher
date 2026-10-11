// atomic/sqlite_io.cpp - 原子化底层实现（SQLite）
#include "atomic/sqlite_io.h"

#include <sqlite3.h>

namespace psig::atomic {

struct SqliteDb {
    sqlite3* db = nullptr;
};

SqliteDb* SqliteOpen(const std::string& path, std::string& err) {
    sqlite3* raw = nullptr;
    int rc = sqlite3_open(path.c_str(), &raw);
    if (rc != SQLITE_OK) {
        err = raw ? sqlite3_errmsg(raw) : "sqlite3_open 失败";
        if (raw) sqlite3_close(raw);
        return nullptr;
    }
    auto* handle = new SqliteDb();
    handle->db = raw;
    return handle;
}

void SqliteClose(SqliteDb* db) {
    if (!db) return;
    if (db->db) sqlite3_close(db->db);
    delete db;
}

bool SqliteExec(SqliteDb* db, const std::string& sql, std::string& err) {
    if (!db || !db->db) { err = "数据库未打开"; return false; }
    char* msg = nullptr;
    int rc = sqlite3_exec(db->db, sql.c_str(), nullptr, nullptr, &msg);
    if (rc != SQLITE_OK) {
        err = msg ? msg : sqlite3_errmsg(db->db);
        if (msg) sqlite3_free(msg);
        return false;
    }
    return true;
}

bool SqliteRun(SqliteDb* db, const std::string& sql,
               const std::vector<std::string>& params, std::string& err) {
    if (!db || !db->db) { err = "数据库未打开"; return false; }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db->db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db->db);
        return false;
    }
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(),
                          static_cast<int>(params[i].size()), SQLITE_TRANSIENT);
    }
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db->db);
        return false;
    }
    return true;
}

bool SqliteQuery(SqliteDb* db, const std::string& sql,
                 std::vector<std::vector<std::string>>& rows, std::string& err) {
    rows.clear();
    if (!db || !db->db) { err = "数据库未打开"; return false; }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db->db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db->db);
        return false;
    }
    int cols = sqlite3_column_count(stmt);
    int rc = SQLITE_ROW;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector<std::string> row;
        row.reserve(static_cast<size_t>(cols));
        for (int c = 0; c < cols; ++c) {
            const unsigned char* text = sqlite3_column_text(stmt, c);
            row.emplace_back(text ? reinterpret_cast<const char*>(text) : "");
        }
        rows.push_back(std::move(row));
    }
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db->db);
        return false;
    }
    return true;
}

bool SqliteTableExists(SqliteDb* db, const std::string& table) {
    if (!db || !db->db) return false;
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db->db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, table.c_str(), static_cast<int>(table.size()), SQLITE_TRANSIENT);
    bool found = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return found;
}

} // namespace psig::atomic
