// test/target/main.cpp - 测试目标程序：在已知地址放置已知值，循环等待被扫描
#include <cstdio>
#include <windows.h>

// 已知特征：一段固定字节序列（放在 .data，地址由链接器确定；ASLR 已关闭）
extern "C" {
volatile unsigned char g_marker[16] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
volatile unsigned int g_counter = 0;
}

int main() {
    const unsigned char* markerPtr = const_cast<const unsigned char*>(g_marker);
    const unsigned int counterValue = g_counter;
    std::printf("========================================\n");
    std::printf(" psig_target : 特征码扫描测试目标程序\n");
    std::printf("========================================\n");
    std::printf("PID: %lu\n", static_cast<unsigned long>(GetCurrentProcessId()));
    std::printf("----------------------------------------\n");
    std::printf("%-16s %-18s %s\n", "变量名", "地址", "值");
    std::printf("%-16s 0x%p   ", "g_marker", static_cast<const void*>(markerPtr));
    for (int i = 0; i < 16; ++i) std::printf("%02X ", markerPtr[i]);
    std::printf("\n");
    std::printf("%-16s 0x%p   %u\n", "g_counter",
                static_cast<const void*>(&counterValue), counterValue);
    std::printf("----------------------------------------\n");
    std::printf("特征码(供扫描): DE AD BE EF 11 22 33 44\n");
    std::fflush(stdout);

    for (;;) {
        g_counter++;
        Sleep(1000);
    }
    return 0;
}
