// atomic/win_icon.cpp - 原子化底层实现（Windows 图标提取 → BGRA 位图）
#include "atomic/win_icon.h"

#include <windows.h>
#include <shellapi.h>

#include <cstring>

namespace psig::atomic {

namespace {

// 将 HICON 转换为 BGRA 位图（自上而下，stride = width*4）
RawBitmap IconToBitmap(HICON hIcon) {
    RawBitmap out;
    if (!hIcon) return out;

    ICONINFO ii{};
    if (!GetIconInfo(hIcon, &ii)) return out;

    BITMAP bm{};
    if (ii.hbmColor) {
        GetObject(ii.hbmColor, sizeof(bm), &bm);
    } else if (ii.hbmMask) {
        GetObject(ii.hbmMask, sizeof(bm), &bm);
        bm.bmHeight /= 2; // 单色掩码高度为实际高度的两倍
    }
    const int w = bm.bmWidth;
    const int h = bm.bmHeight;

    auto cleanupHandles = [&]() {
        if (ii.hbmColor) DeleteObject(ii.hbmColor);
        if (ii.hbmMask) DeleteObject(ii.hbmMask);
    };
    if (w <= 0 || h <= 0) {
        cleanupHandles();
        return out;
    }

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h; // 负高度 = 自上而下
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = CreateCompatibleDC(nullptr);
    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!dib || !bits) {
        if (dib) DeleteObject(dib);
        if (hdc) DeleteDC(hdc);
        cleanupHandles();
        return out;
    }
    HGDIOBJ old = SelectObject(hdc, dib);
    DrawIconEx(hdc, 0, 0, hIcon, w, h, 0, nullptr, DI_NORMAL);

    const size_t npix = static_cast<size_t>(w) * h;
    out.width = w;
    out.height = h;
    out.pixels.assign(npix * 4, 0);
    std::memcpy(out.pixels.data(), bits, npix * 4);

    // 若无 alpha 通道（全 0），用 AND 掩码重建：掩码 0=不透明，1=透明
    bool anyAlpha = false;
    for (size_t i = 3; i < out.pixels.size(); i += 4) {
        if (out.pixels[i] != 0) { anyAlpha = true; break; }
    }
    if (!anyAlpha && ii.hbmMask) {
        void* maskBits = nullptr;
        HBITMAP maskDib = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &maskBits, nullptr, 0);
        if (maskDib && maskBits) {
            HGDIOBJ oldMask = SelectObject(hdc, maskDib);
            DrawIconEx(hdc, 0, 0, hIcon, w, h, 0, nullptr, DI_MASK);
            SelectObject(hdc, oldMask);
            const uint8_t* mb = static_cast<const uint8_t*>(maskBits);
            for (size_t p = 0; p < npix; ++p) {
                out.pixels[p * 4 + 3] = (mb[p * 4] == 0) ? 255 : 0;
            }
        }
        if (maskDib) DeleteObject(maskDib);
    }

    SelectObject(hdc, old);
    DeleteObject(dib);
    DeleteDC(hdc);
    cleanupHandles();
    return out;
}

} // namespace

RawBitmap ExtractProcessIcon(uint32_t pid) {
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return {};
    wchar_t buf[MAX_PATH * 2];
    DWORD size = static_cast<DWORD>(sizeof(buf) / sizeof(buf[0]));
    const bool ok = QueryFullProcessImageNameW(h, 0, buf, &size) != 0;
    CloseHandle(h);
    if (!ok) return {};

    SHFILEINFOW shfi{};
    if (!SHGetFileInfoW(buf, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_LARGEICON))
        return {};
    RawBitmap bmp = IconToBitmap(shfi.hIcon);
    if (shfi.hIcon) DestroyIcon(shfi.hIcon);
    return bmp;
}

RawBitmap ExtractDefaultIcon() {
    SHFILEINFOW shfi{};
    if (!SHGetFileInfoW(L"a.exe", FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
                        SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES))
        return {};
    RawBitmap bmp = IconToBitmap(shfi.hIcon);
    if (shfi.hIcon) DestroyIcon(shfi.hIcon);
    return bmp;
}

} // namespace psig::atomic
