// psig.h - 公共 API（进程特征码批量匹配静态库）
// 版本 v0.1.0
// 说明：本头文件是 psig 静态库唯一对外契约；GUI 只能经此公共 API 调用静态库能力。
// 约束：静态库不依赖 Qt；所有公共 API 不抛异常，用返回值/出参表达错误；线程安全。
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace psig {

// ---------------------------------------------------------------------------
// 基础数据结构
// ---------------------------------------------------------------------------

// 进程信息
struct ProcessInfo {
    uint32_t pid = 0;
    std::wstring name;      // 进程名（如 notepad.exe）
    std::wstring path;      // 可执行文件完整路径（可能为空）
    bool accessible = false; // 是否可用读权限打开（决定能否扫描）
};

// 特征码模式（解析后）
struct Pattern {
    std::string name;            // 名称
    std::string raw;             // 原始模式串
    std::vector<uint8_t> bytes;  // 期望字节
    std::vector<uint8_t> mask;   // 掩码：0xFF 全匹配，0x00 全通配，0xF0/0x0F 半字节
    bool valid = false;          // 是否合法
    std::string error;           // 非法原因
};

// 一条匹配结果
struct MatchResult {
    std::string patternName;     // 命中的特征码名称
    uint32_t pid = 0;            // 进程 pid
    std::wstring processName;    // 进程名
    uint64_t address = 0;        // 命中虚拟地址
    std::wstring moduleName;     // 命中所在模块名（可能为空）
    uint64_t moduleBase = 0;     // 模块基址
    std::string matchedBytes;    // 命中处字节（hex 串）
    std::string hitTime;         // 命中时间 ISO8601
};

// 动态列表格：列定义
struct ColumnDef {
    std::string key;      // 唯一键
    std::string title;    // 显示标题
    std::string type;     // text / int / enum
    bool builtin = false; // 内置列（不可删除）
};

// 动态列表格：行
struct TableRow {
    std::vector<std::string> cells; // 与 columns 一一对应
};

// 动态列表格
struct Table {
    std::vector<ColumnDef> columns;
    std::vector<TableRow> rows;
};

// 扫描进度
struct ScanProgress {
    uint32_t currentPid = 0;
    size_t done = 0;    // 已处理模式数
    size_t total = 0;   // 总模式数
    size_t hits = 0;    // 当前命中累计
};

// 进度回调（在后台线程触发；GUI 侧应经 QueuedConnection 回主线程）
using ProgressCallback = std::function<void(const ScanProgress&)>;

// 扫描请求
struct ScanRequest {
    std::vector<uint32_t> pids;         // 目标进程
    std::vector<Pattern> patterns;      // 特征码
    size_t maxResultsPerPattern = 0;    // 每模式命中上限，0 表示不限
};

// ---------------------------------------------------------------------------
// 进程 / 扫描 API
// ---------------------------------------------------------------------------

// 枚举系统进程
std::vector<ProcessInfo> EnumerateProcesses();

// 解析模式串（支持 '48 8B ?? 05'、'488B??05'、'?8 8B A?' 等）
Pattern ParsePattern(const std::string& raw, const std::string& name);

// 批量扫描：对 req.pids × req.patterns 做内存特征匹配
// progress 可为空；cancel 可为空，非空时轮询取消
std::vector<MatchResult> ScanProcesses(const ScanRequest& req,
                                       ProgressCallback progress = nullptr,
                                       const std::atomic_bool* cancel = nullptr);

// 请求取消当前扫描（全局标志；与 ScanProcesses 的 cancel 出参二选一使用）
void RequestCancel(bool flag);

// 内置特征码表列定义（name/pattern/note）
std::vector<ColumnDef> BuiltinSignatureColumns();

// ---------------------------------------------------------------------------
// 数据交换 API（特征码表 / 匹配结果）
// ---------------------------------------------------------------------------

// 导入特征码表（CSV / SQLite .db）；失败时置 err 并返回空表
Table ImportSignaturesCsv(const std::string& path, std::string& err);
Table ImportSignaturesDb(const std::string& path, std::string& err);

// 导出特征码表；失败返回 false 并置 err
bool ExportSignaturesCsv(const std::string& path, const Table& table, std::string& err);
bool ExportSignaturesDb(const std::string& path, const Table& table, std::string& err);

// 导出匹配结果（CSV / SQLite .db / TXT）；失败返回 false 并置 err
bool ExportResultsCsv(const std::string& path, const std::vector<MatchResult>& results, std::string& err);
bool ExportResultsDb(const std::string& path, const std::vector<MatchResult>& results, std::string& err);
bool ExportResultsTxt(const std::string& path, const std::vector<MatchResult>& results, std::string& err);

} // namespace psig
