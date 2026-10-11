// test/integration/test_scan.cpp - 集成测试：真实进程内存扫描 + 数据交换
#include <gtest/gtest.h>

#include <cstdio>
#include <string>
#include <vector>

#include <windows.h>

#include "psig.h"

#ifndef PSIG_TARGET_PATH
#define PSIG_TARGET_PATH ""
#endif

namespace {

// 本测试进程内的已知特征（用于扫描自身）
extern "C" {
volatile unsigned char g_self_marker[16] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
}

// 启动目标程序并读取其 PID
bool SpawnTarget(const std::string& exe, PROCESS_INFORMATION& pi, uint32_t& pid) {
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    HANDLE rd = nullptr, wr = nullptr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) return false;
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = wr;
    si.hStdError = wr;
    std::string cmd = exe;
    if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        CloseHandle(rd);
        CloseHandle(wr);
        return false;
    }
    CloseHandle(wr);
    // 读取输出直到找到 PID
    std::string out;
    char buf[512];
    DWORD n = 0;
    for (int i = 0; i < 50; ++i) {
        if (ReadFile(rd, buf, sizeof(buf) - 1, &n, nullptr) && n > 0) {
            buf[n] = 0;
            out += buf;
            if (out.find("PID:") != std::string::npos) break;
        } else {
            Sleep(50);
        }
    }
    CloseHandle(rd);
    size_t pos = out.find("PID:");
    if (pos == std::string::npos) return false;
    pid = static_cast<uint32_t>(std::strtoul(out.c_str() + pos + 4, nullptr, 10));
    return pid != 0;
}

std::string TempPath(const char* name) {
    char dir[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, dir);
    std::string s(dir, n);
    s += name;
    return s;
}

} // namespace

// 枚举进程：应包含自身
TEST(Integration, EnumerateContainsSelf) {
    auto procs = psig::EnumerateProcesses();
    ASSERT_FALSE(procs.empty());
    uint32_t self = GetCurrentProcessId();
    bool found = false;
    for (const auto& p : procs) {
        if (p.pid == self) found = true;
    }
    EXPECT_TRUE(found);
}

// 扫描自身：应命中本进程内的已知特征
TEST(Integration, ScanSelfFindsMarker) {
    psig::ScanRequest req;
    req.pids = {GetCurrentProcessId()};
    req.patterns = {psig::ParsePattern("DE AD BE EF 11 22 33 44", "self_marker")};
    ASSERT_TRUE(req.patterns[0].valid);
    auto results = psig::ScanProcesses(req, nullptr, nullptr);
    EXPECT_GE(results.size(), 1u);
    for (const auto& r : results) {
        EXPECT_EQ(r.patternName, "self_marker");
        EXPECT_EQ(r.pid, GetCurrentProcessId());
    }
}

// 跨进程扫描：启动 psig_target.exe 并命中其 g_marker
TEST(Integration, ScanExternalTarget) {
    if (std::string(PSIG_TARGET_PATH).empty()) {
        GTEST_SKIP() << "未提供 PSIG_TARGET_PATH";
    }
    PROCESS_INFORMATION pi{};
    uint32_t pid = 0;
    ASSERT_TRUE(SpawnTarget(PSIG_TARGET_PATH, pi, pid)) << "无法启动目标程序";
    Sleep(300);

    psig::ScanRequest req;
    req.pids = {pid};
    req.patterns = {psig::ParsePattern("DE AD BE EF 11 22 33 44", "target_marker")};
    auto results = psig::ScanProcesses(req, nullptr, nullptr);

    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    EXPECT_GE(results.size(), 1u);
    if (!results.empty()) {
        EXPECT_EQ(results[0].patternName, "target_marker");
        EXPECT_GT(results[0].address, 0u);
    }
}

// 特征码表 CSV 往返
TEST(Integration, SignatureCsvRoundTrip) {
    psig::Table t;
    t.columns = psig::BuiltinSignatureColumns();
    t.rows = {{{"p1", "48 8B ?? 05", "note1"}}, {{"p2", "DE AD BE EF", "note2"}}};
    std::string err;
    std::string path = TempPath("psig_sig.csv");
    ASSERT_TRUE(psig::ExportSignaturesCsv(path, t, err)) << err;
    psig::Table back = psig::ImportSignaturesCsv(path, err);
    ASSERT_EQ(back.rows.size(), 2u) << err;
    EXPECT_EQ(back.rows[1].cells[1], "DE AD BE EF");
    DeleteFileA(path.c_str());
}

// 特征码表 DB 往返
TEST(Integration, SignatureDbRoundTrip) {
    psig::Table t;
    t.columns = psig::BuiltinSignatureColumns();
    t.columns.push_back({"custom", "自定义列", "text", false});
    t.rows = {{{"p1", "48 8B ?? 05", "n1", "v1"}}};
    std::string err;
    std::string path = TempPath("psig_sig.db");
    DeleteFileA(path.c_str());
    ASSERT_TRUE(psig::ExportSignaturesDb(path, t, err)) << err;
    psig::Table back = psig::ImportSignaturesDb(path, err);
    ASSERT_EQ(back.columns.size(), 4u) << err;
    ASSERT_EQ(back.rows.size(), 1u);
    EXPECT_EQ(back.rows[0].cells[3], "v1");
    DeleteFileA(path.c_str());
}

// 匹配结果三格式导出
TEST(Integration, ExportResultsFormats) {
    std::vector<psig::MatchResult> results;
    psig::MatchResult m;
    m.patternName = "p1";
    m.pid = 1234;
    m.processName = L"dummy.exe";
    m.address = 0x140001000;
    m.moduleName = L"dummy.exe";
    m.moduleBase = 0x140000000;
    m.matchedBytes = "DE AD BE EF";
    m.hitTime = "2026-10-11T12:00:00";
    results.push_back(m);

    std::string err;
    std::string csv = TempPath("psig_res.csv");
    std::string db = TempPath("psig_res.db");
    std::string txt = TempPath("psig_res.txt");
    DeleteFileA(db.c_str());
    EXPECT_TRUE(psig::ExportResultsCsv(csv, results, err)) << err;
    EXPECT_TRUE(psig::ExportResultsDb(db, results, err)) << err;
    EXPECT_TRUE(psig::ExportResultsTxt(txt, results, err)) << err;

    std::string content;
    HANDLE h = CreateFileA(txt.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        char buf[1024];
        DWORD n = 0;
        if (ReadFile(h, buf, sizeof(buf) - 1, &n, nullptr)) { buf[n] = 0; content = buf; }
        CloseHandle(h);
    }
    EXPECT_NE(content.find("dummy.exe"), std::string::npos);
    DeleteFileA(csv.c_str());
    DeleteFileA(db.c_str());
    DeleteFileA(txt.c_str());
}
