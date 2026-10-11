# 18 原子化 api 设计（psig v0.2.1）

> 前置：`17_原子化api分析.md`

## 新增原子化 API（`atomic/win_icon.h/.cpp`）

```cpp
// atomic/win_icon.h
#include <cstdint>
#include <vector>

namespace psig::atomic {

// 位图结果（内部表示；接口层转换为公共 IconBitmap）
struct RawBitmap {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels; // BGRA，自上而下
};

// 提取指定进程的图标；失败返回 width==0
RawBitmap ExtractProcessIcon(uint32_t pid);

// 系统默认可执行文件图标；通常有效
RawBitmap ExtractDefaultIcon();

} // namespace psig::atomic
```

## 复用既有原子化 API

```cpp
// atomic/file_io.h（既有）
bool ReadTextFile (const std::string& path, std::string& out);
bool WriteTextFile(const std::string& path, const std::string& text);
```

## 实现要点（Win32）

1. `ExtractProcessIcon`：`OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION)` → `QueryFullProcessImageNameW` 取路径；失败返回空。
2. `SHGetFileInfoW(path, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_LARGEICON)` 取 `shfi.hIcon`；失败返回空。
3. `HIconToBitmap(hIcon)`：`GetIconInfo` → `GetObject` 取尺寸 → 建 32bpp DIB → `DrawIconEx`/`GetDIBits` 得到 BGRA；无 alpha 时用掩码合成。
4. `DestroyIcon(hIcon)` 释放。
5. `ExtractDefaultIcon`：对 `L"a.exe"` 用 `SHGFI_USEFILEATTRIBUTES` 取默认 exe 图标。

## 链接

- `shell32`（SHGetFileInfo）、`user32`（DrawIconEx）、`gdi32`（GetDIBits）。

## 约束满足说明

- 原子化 API 设计保持最小、通用、无业务语义；平台细节封装在本层。
