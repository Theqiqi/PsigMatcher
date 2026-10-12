// psig.h - 公共 API（进程特征码批量匹配静态库）
// 版本 v0.2.4
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

// 图标位图（BGRA，小端 ARGB32，自上而下，stride = width*4）
// 可直接映射 Qt QImage::Format_ARGB32；width==0 表示无有效图标。
struct IconBitmap {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels; // 长度 = width*height*4
    bool valid() const {
        return width > 0 && height > 0 &&
               pixels.size() == static_cast<size_t>(width) * height * 4;
    }
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
    std::string custom;          // 自定义列文本（可空，随导出携带）
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
    std::string templateText;    // 表级模板文本（多行，可空；随导入/导出携带）
};

// 匹配结果导出选项
struct ResultExportOptions {
    std::string templateText;            // 表首模板文本（可空；TXT 置于最前 / CSV 前置记录 / DB 元数据）
    std::vector<std::string> columns;    // 要导出的结果列键（空=全部，不含 address_dec）。键见 psig 结果列：pattern_name/process_name/pid/address/module_name/module_base/matched_bytes/hit_time/custom/address_dec
    std::vector<std::string> headers;    // 与 columns 一一对应的表头文字（可空）；非空时优先于默认中文表头（用于跟随界面语言）。长度需与 columns 一致
    std::string summary;                 // TXT 摘要行文案（可空）；非空时替换默认中文摘要行（用于跟随界面语言）
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

// 提取指定进程可执行文件的图标；取不到返回 valid()==false 的空位图（不抛异常）
IconBitmap ProcessIcon(uint32_t pid);

// 系统默认可执行文件图标（供无图标进程回退显示）；通常 valid()==true
IconBitmap DefaultProcessIcon();

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
// CSV 导入会识别并剥离文件头 UTF-8 BOM（若有）。
Table ImportSignaturesCsv(const std::string& path, std::string& err);
Table ImportSignaturesDb(const std::string& path, std::string& err);

// 导出特征码表；失败返回 false 并置 err
// CSV 导出会在文件最前写入 UTF-8 BOM（使 Excel 等正确解析中文表头）。
bool ExportSignaturesCsv(const std::string& path, const Table& table, std::string& err);
bool ExportSignaturesDb(const std::string& path, const Table& table, std::string& err);

// 导出匹配结果（CSV / SQLite .db / TXT）；失败返回 false 并置 err
// CSV 导出会在文件最前写入 UTF-8 BOM。
// options 可省略（默认全部列、无模板）；TXT 摘要行优先取 options.summary（非空），否则默认中文。
bool ExportResultsCsv(const std::string& path, const std::vector<MatchResult>& results,
                      std::string& err, const ResultExportOptions& options = {});
bool ExportResultsDb(const std::string& path, const std::vector<MatchResult>& results,
                     std::string& err, const ResultExportOptions& options = {});
bool ExportResultsTxt(const std::string& path, const std::vector<MatchResult>& results,
                      std::string& err, const ResultExportOptions& options = {});

// ---------------------------------------------------------------------------
// 结果列查询 API（供界面按当前语言构造导出表头）
// ---------------------------------------------------------------------------

// 结果列定义：(键, 默认中文标题)，顺序即导出顺序
std::vector<std::pair<std::string, std::string>> ResultColumnDefs();

// 解析实际导出的结果列键：requested 为空/全未知 → 默认集合（不含 address_dec）；
// 未知键被忽略；返回顺序与 requested 一致（address_dec 需显式请求）
std::vector<std::string> SelectResultKeys(const std::vector<std::string>& requested);

} // namespace psig
