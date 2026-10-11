// atomic/sqlite_io.h - 原子化底层：SQLite 原语（.db 读写）
#pragma once

#include <string>
#include <vector>

namespace psig::atomic {

// 不透明数据库句柄（sqlite3* 细节不外泄）
struct SqliteDb;

// 打开/创建数据库；失败返回 nullptr 并置 err
SqliteDb* SqliteOpen(const std::string& path, std::string& err);

// 关闭数据库
void SqliteClose(SqliteDb* db);

// 执行无返回的 SQL（建表、DELETE 等）；失败置 err
bool SqliteExec(SqliteDb* db, const std::string& sql, std::string& err);

// 执行带 ? 参数的写语句（INSERT 等）
bool SqliteRun(SqliteDb* db, const std::string& sql,
               const std::vector<std::string>& params, std::string& err);

// 查询：返回每行为字符串列；失败置 err
bool SqliteQuery(SqliteDb* db, const std::string& sql,
                 std::vector<std::vector<std::string>>& rows, std::string& err);

// 查询表是否存在
bool SqliteTableExists(SqliteDb* db, const std::string& table);

} // namespace psig::atomic
