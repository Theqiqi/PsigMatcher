// test/target/main.cpp - GUI 集成/E2E 测试目标程序（已知特征，PID 输出）
#include <cstdio>
#include <windows.h>

extern "C" {
volatile unsigned char g_gui_marker[16] = {
    0xCA, 0xFE, 0xBA, 0xBE, 0x13, 0x37, 0x42, 0x42,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
volatile unsigned int g_gui_counter = 0;
}

int main() {
    std::printf("========================================\n");
    std::printf(" psig_target : GUI 测试目标程序\n");
    std::printf("========================================\n");
    std::printf("PID: %lu\n", static_cast<unsigned long>(GetCurrentProcessId()));
    std::printf("----------------------------------------\n");
    std::printf("g_gui_marker @ 0x%p\n", static_cast<const void*>(const_cast<const unsigned char*>(g_gui_marker)));
    std::printf("特征码: CA FE BA BE 13 37 42 42\n");
    std::printf("----------------------------------------\n");
    std::fflush(stdout);
    for (;;) {
        g_gui_counter++;
        Sleep(1000);
    }
    return 0;
}
