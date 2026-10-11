// test/integration/test_scan.cpp - 集成测试：真实进程内存扫描 + 数据交换
#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
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

namespace {
std::string ReadAll(const std::string& path) {
    std::string content;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        char buf[4096];
        DWORD n = 0;
        while (ReadFile(h, buf, sizeof(buf) - 1, &n, nullptr) && n > 0) {
            buf[n] = 0;
            content += buf;
        }
        CloseHandle(h);
    }
    return content;
}
} // namespace

// 特征码表 CSV 携带模板文本往返
TEST(Integration, SignatureCsvTemplateRoundTrip) {
    psig::Table t;
    t.columns = psig::BuiltinSignatureColumns();
    t.rows = {{{"p1", "48 8B ?? 05", "n1"}}};
    t.templateText = "多行模板\n含,逗号 与 \"引号\"";
    std::string err;
    std::string path = TempPath("psig_sig_tmpl.csv");
    ASSERT_TRUE(psig::ExportSignaturesCsv(path, t, err)) << err;
    psig::Table back = psig::ImportSignaturesCsv(path, err);
    ASSERT_EQ(back.rows.size(), 1u) << err;
    EXPECT_EQ(back.templateText, t.templateText);
    EXPECT_EQ(back.rows[0].cells[1], "48 8B ?? 05");
    DeleteFileA(path.c_str());
}

// 特征码表 DB 携带模板文本往返
TEST(Integration, SignatureDbTemplateRoundTrip) {
    psig::Table t;
    t.columns = psig::BuiltinSignatureColumns();
    t.rows = {{{"p1", "DE AD", "n1"}}};
    t.templateText = "DB模板\n第二行";
    std::string err;
    std::string path = TempPath("psig_sig_tmpl.db");
    DeleteFileA(path.c_str());
    ASSERT_TRUE(psig::ExportSignaturesDb(path, t, err)) << err;
    psig::Table back = psig::ImportSignaturesDb(path, err);
    ASSERT_EQ(back.rows.size(), 1u) << err;
    EXPECT_EQ(back.templateText, t.templateText);
    DeleteFileA(path.c_str());
}

// 旧 CSV（无模板记录）导入不报错且模板为空
TEST(Integration, SignatureCsvNoTemplateCompat) {
    std::string path = TempPath("psig_sig_old.csv");
    HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    ASSERT_NE(h, INVALID_HANDLE_VALUE);
    const char* body = "名称,特征码,备注\r\np1,AA BB,n\r\n";
    DWORD written = 0;
    WriteFile(h, body, static_cast<DWORD>(std::strlen(body)), &written, nullptr);
    CloseHandle(h);

    std::string err;
    psig::Table back = psig::ImportSignaturesCsv(path, err);
    EXPECT_TRUE(err.empty()) << err;
    ASSERT_EQ(back.rows.size(), 1u);
    EXPECT_TRUE(back.templateText.empty());
    DeleteFileA(path.c_str());
}

// 结果导出：模板文本 + 自定义列 + 选择列子集
TEST(Integration, ExportResultsOptionsAndCustom) {
    std::vector<psig::MatchResult> results;
    psig::MatchResult m;
    m.patternName = "p1";
    m.pid = 4321;
    m.processName = L"opt.exe";
    m.address = 0x140002000;
    m.moduleName = L"opt.exe";
    m.moduleBase = 0x140000000;
    m.matchedBytes = "AA BB";
    m.hitTime = "2026-10-11T13:00:00";
    m.custom = "我的自定义值";
    results.push_back(m);

    psig::ResultExportOptions opt;
    opt.templateText = "结果模板\n行2";
    opt.columns = {"pid", "custom"};

    std::string err;
    std::string csv = TempPath("psig_res_opt.csv");
    std::string db = TempPath("psig_res_opt.db");
    std::string txt = TempPath("psig_res_opt.txt");
    DeleteFileA(db.c_str());
    ASSERT_TRUE(psig::ExportResultsCsv(csv, results, err, opt)) << err;
    ASSERT_TRUE(psig::ExportResultsDb(db, results, err, opt)) << err;
    ASSERT_TRUE(psig::ExportResultsTxt(txt, results, err, opt)) << err;

    const std::string csvText = ReadAll(csv);
    EXPECT_NE(csvText.find("#psig-template"), std::string::npos);
    EXPECT_NE(csvText.find("我的自定义值"), std::string::npos);
    EXPECT_NE(csvText.find("PID"), std::string::npos);
    EXPECT_NE(csvText.find("备注"), std::string::npos);
    EXPECT_EQ(csvText.find("特征码名"), std::string::npos); // 未选列不导出

    const std::string txtText = ReadAll(txt);
    EXPECT_EQ(txtText.rfind("结果模板", 0), 0u); // 模板位于文件最前

    DeleteFileA(csv.c_str());
    DeleteFileA(db.c_str());
    DeleteFileA(txt.c_str());
}

// CSV 导出写 UTF-8 BOM（特征码表与结果）
TEST(Integration, CsvExportHasUtf8Bom) {
    psig::Table t;
    t.columns = psig::BuiltinSignatureColumns();
    t.rows = {{{"p1", "AA BB", "n1"}}};
    std::string err;
    std::string sigCsv = TempPath("psig_bom_sig.csv");
    ASSERT_TRUE(psig::ExportSignaturesCsv(sigCsv, t, err)) << err;
    const std::string sig = ReadAll(sigCsv);
    ASSERT_GE(sig.size(), 3u);
    EXPECT_EQ(static_cast<unsigned char>(sig[0]), 0xEFu);
    EXPECT_EQ(static_cast<unsigned char>(sig[1]), 0xBBu);
    EXPECT_EQ(static_cast<unsigned char>(sig[2]), 0xBFu);
    DeleteFileA(sigCsv.c_str());

    std::vector<psig::MatchResult> results;
    psig::MatchResult m;
    m.patternName = "p1";
    m.pid = 1;
    m.matchedBytes = "AA BB";
    results.push_back(m);
    std::string resCsv = TempPath("psig_bom_res.csv");
    ASSERT_TRUE(psig::ExportResultsCsv(resCsv, results, err)) << err;
    const std::string res = ReadAll(resCsv);
    ASSERT_GE(res.size(), 3u);
    EXPECT_EQ(static_cast<unsigned char>(res[0]), 0xEFu);
    EXPECT_EQ(static_cast<unsigned char>(res[1]), 0xBBu);
    EXPECT_EQ(static_cast<unsigned char>(res[2]), 0xBFu);
    DeleteFileA(resCsv.c_str());
}

// 带 BOM 的 CSV 往返：首列表头不被 BOM 污染
TEST(Integration, CsvBomRoundTripHeaderClean) {
    psig::Table t;
    t.columns = psig::BuiltinSignatureColumns(); // 首列表头为「名称」
    t.rows = {{{"p1", "AA BB", "n1"}}};
    std::string err;
    std::string path = TempPath("psig_bom_rt.csv");
    ASSERT_TRUE(psig::ExportSignaturesCsv(path, t, err)) << err;
    psig::Table back = psig::ImportSignaturesCsv(path, err);
    ASSERT_EQ(back.columns.size(), 3u) << err;
    EXPECT_EQ(back.columns[0].title, "名称");
    ASSERT_EQ(back.rows.size(), 1u);
    EXPECT_EQ(back.rows[0].cells[0], "p1");
    DeleteFileA(path.c_str());
}

// R-01：结果导出支持十进制地址列（address_dec）
TEST(Integration, ExportResultsDecimalAddress) {
    std::vector<psig::MatchResult> results;
    psig::MatchResult m;
    m.patternName = "p1";
    m.pid = 100;
    m.address = 0x140001234; // 十进制 5368721972
    results.push_back(m);

    // 默认全列不含 address_dec
    psig::ResultExportOptions def;
    std::string err;
    std::string csvDef = TempPath("psig_dec_def.csv");
    ASSERT_TRUE(psig::ExportResultsCsv(csvDef, results, err, def)) << err;
    std::string defText = ReadAll(csvDef);
    EXPECT_EQ(defText.find("地址(十进制)"), std::string::npos); // 默认不含
    DeleteFileA(csvDef.c_str());

    // 显式选择 address_dec → 十进制值与表头
    psig::ResultExportOptions opt;
    opt.columns = {"pid", "address_dec"};
    std::string csv = TempPath("psig_dec.csv");
    ASSERT_TRUE(psig::ExportResultsCsv(csv, results, err, opt)) << err;
    std::string text = ReadAll(csv);
    EXPECT_NE(text.find("地址(十进制)"), std::string::npos);
    EXPECT_NE(text.find(std::to_string(m.address)), std::string::npos); // 十进制值
    EXPECT_EQ(text.find("0x"), std::string::npos); // 不含十六进制
    DeleteFileA(csv.c_str());
}

// R-02：结果导出表头可随调用方语言（options.headers 优先）
TEST(Integration, ExportResultsHeadersOverride) {
    std::vector<psig::MatchResult> results;
    psig::MatchResult m;
    m.patternName = "p1";
    m.pid = 200;
    results.push_back(m);

    psig::ResultExportOptions opt;
    opt.columns = {"pid", "process_name", "address"};
    opt.headers = {"PID", "Process", "Address"};
    std::string err;
    std::string csv = TempPath("psig_hdr.csv");
    ASSERT_TRUE(psig::ExportResultsCsv(csv, results, err, opt)) << err;
    std::string text = ReadAll(csv);
    EXPECT_NE(text.find("PID"), std::string::npos);
    EXPECT_NE(text.find("Process"), std::string::npos);
    EXPECT_EQ(text.find("特征码名"), std::string::npos); // 未用中文默认
    DeleteFileA(csv.c_str());
}

// R-04：特征码导入支持英文标题回映射（跨语言往返）
TEST(Integration, SignatureImportEnglishTitles) {
    psig::Table t;
    t.columns = {
        {"name", "Name", "text", true},
        {"pattern", "Signature", "text", true},
        {"note", "Remarks", "text", true},
    };
    t.rows = {{{"p1", "AA BB", "n1"}}};
    std::string err;
    std::string path = TempPath("psig_sig_en.csv");
    ASSERT_TRUE(psig::ExportSignaturesCsv(path, t, err)) << err;
    psig::Table back = psig::ImportSignaturesCsv(path, err);
    ASSERT_EQ(back.rows.size(), 1u) << err;
    ASSERT_EQ(back.columns.size(), 3u);
    EXPECT_EQ(back.columns[0].key, "name");
    EXPECT_EQ(back.columns[1].key, "pattern");
    EXPECT_EQ(back.columns[2].key, "note");
    DeleteFileA(path.c_str());
}

// 进程图标 API：默认图标有效；不存在的 pid 返回 invalid 且不崩溃
TEST(Integration, ProcessIconApis) {
    psig::IconBitmap def = psig::DefaultProcessIcon();
    EXPECT_TRUE(def.valid());
    EXPECT_GT(def.width, 0);
    EXPECT_GT(def.height, 0);
    EXPECT_EQ(def.pixels.size(), static_cast<size_t>(def.width) * def.height * 4);

    // 不存在的 pid → invalid（不抛异常）
    psig::IconBitmap none = psig::ProcessIcon(0xFFFFFFFEu);
    EXPECT_FALSE(none.valid());

    // 自身进程：不要求一定有图标，但调用不应崩溃；若有效则尺寸一致
    psig::IconBitmap self = psig::ProcessIcon(GetCurrentProcessId());
    if (self.valid()) {
        EXPECT_EQ(self.pixels.size(), static_cast<size_t>(self.width) * self.height * 4);
    }
}
