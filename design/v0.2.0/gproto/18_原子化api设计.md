# 18 原子化 api 设计（psig v0.2.0）

> 前置：`17_原子化api分析.md`

## 结论：无新增原子化 API

本次改动复用既有原子化 API：

```cpp
// atomic/sqlite_io.h（既有）
SqliteDb* SqliteOpen(const std::string& path, std::string& err);
void      SqliteClose(SqliteDb* db);
bool      SqliteExec (SqliteDb* db, const std::string& sql, std::string& err);
bool      SqliteRun  (SqliteDb* db, const std::string& sql, const std::vector<std::string>& params, std::string& err);
bool      SqliteQuery(SqliteDb* db, const std::string& sql, std::vector<std::vector<std::string>>& rows, std::string& err);
bool      SqliteTableExists(SqliteDb* db, const std::string& table);

// atomic/file_io.h（既有）
bool ReadTextFile (const std::string& path, std::string& out);
bool WriteTextFile(const std::string& path, const std::string& text);
```

## 约束满足说明

- 原子化 API 设计保持最小、通用、无业务语义。
