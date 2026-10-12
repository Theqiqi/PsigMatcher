#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""check_exe_icon.py -- 校验编译产物 exe 的 PE 图标资源（产物侧，与源侧 check_app_icon.py 对应）。

校验项（任一不满足即非零退出码）：
  1. 目标文件是 PE 且存在资源目录（IMAGE_DIRECTORY_ENTRY_RESOURCE）；
  2. 资源目录中存在 RT_GROUP_ICON (14) 与 RT_ICON (3)；
  3. 组图标声明的条目数 == 7，且尺寸集合 == {16,24,32,48,64,128,256}；
  4. 每个 RT_ICON 载荷与 gui/res/app.ico 中对应尺寸帧的字节一致（逐字节比对）。

可选第 5 项（--shell-icon <png>）：比对「Windows 外壳关联到该 exe 的图标」与 app_32.png 的像素。
  该 PNG 由 gui/script/verify/extract_exe_icon.ps1 生成（走 System.Drawing.Icon::ExtractAssociatedIcon，
  即资源管理器显示文件图标时的同一套关联逻辑），由此提供独立于本脚本 PE 解析的证据。

用法：python3 gui/script/verify/check_exe_icon.py [exe 路径] [--shell-icon <png>]
      默认 exe = gui/out/bin/Release/PsigMatcher.exe
依赖：仅标准库（不依赖 Pillow；--shell-icon 模式用 PNG 内部结构逐像素比对）。
"""

from __future__ import annotations

import struct
import sys
import zlib
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
GUI_DIR = SCRIPT_DIR.parent.parent
DEFAULT_EXE = GUI_DIR / "out" / "bin" / "Release" / "PsigMatcher.exe"
RES_DIR = GUI_DIR / "res"
ICO = RES_DIR / "app.ico"

RT_ICON = 3
RT_GROUP_ICON = 14
EXPECTED_SIZES = (16, 24, 32, 48, 64, 128, 256)


def read_ico_frames(path: Path) -> dict:
    """解析 ICO，返回 {尺寸: 载荷字节}。"""
    data = path.read_bytes()
    reserved, icon_type, count = struct.unpack_from("<HHH", data, 0)
    if reserved != 0 or icon_type != 1:
        raise ValueError("不是 ICO 文件：%s" % path)
    frames = {}
    for index in range(count):
        base = 6 + 16 * index
        width, height, _colors, _res, _planes, _bits, size, offset = struct.unpack_from("<BBBBHHII", data, base)
        dimension = width if width else 256
        frames[(dimension, height if height else 256)] = data[offset:offset + size]
    return frames


def pe_resources(data: bytes) -> dict:
    """返回 PE 资源目录的 {类型: {名称: 载荷字节}}（仅一层类型/一层名称）。"""
    if data[:2] != b"MZ":
        raise ValueError("不是 PE 文件（缺少 MZ 头）")
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe_offset:pe_offset + 4] != b"PE\0\0":
        raise ValueError("不是 PE 文件（缺少 PE 签名）")
    section_count = struct.unpack_from("<H", data, pe_offset + 6)[0]
    optional_header_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
    optional = pe_offset + 24
    magic = struct.unpack_from("<H", data, optional)[0]
    if magic != 0x20B:
        raise ValueError("仅支持 PE32+（x64）产物，magic=0x%X" % magic)
    data_directories = optional + 112
    resource_rva, resource_size = struct.unpack_from("<II", data, data_directories + 2 * 8)
    if resource_rva == 0 or resource_size == 0:
        return {}

    sections = []
    section_table = optional + optional_header_size
    for index in range(section_count):
        base = section_table + 40 * index
        virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from("<IIII", data, base + 8)
        sections.append((virtual_address, virtual_size, raw_offset, raw_size))

    def to_offset(rva: int) -> int:
        for virtual_address, virtual_size, raw_offset, raw_size in sections:
            if virtual_address <= rva < virtual_address + max(virtual_size, raw_size):
                return raw_offset + (rva - virtual_address)
        raise ValueError("RVA 0x%X 不在任何节内" % rva)

    root = to_offset(resource_rva)
    result = {}

    def walk(directory_offset: int, depth: int, path: list) -> None:
        # IMAGE_RESOURCE_DIRECTORY：具名条目在前、ID 条目在后，各 8 字节（Name/ID + OffsetToData）
        named, id_count = struct.unpack_from("<HH", data, directory_offset + 12)
        for index in range(named, named + id_count):
            entry = directory_offset + 16 + 8 * index
            name_id, offset = struct.unpack_from("<II", data, entry)
            is_directory = bool(offset & 0x80000000)
            if depth < 2:
                if not is_directory:
                    continue  # 类型层/名称层必为子目录
                walk(to_offset(resource_rva + (offset & 0x7FFFFFFF)), depth + 1, path + [name_id])
            else:
                if is_directory:
                    continue  # 语言层必为叶子（IMAGE_RESOURCE_DATA_ENTRY）
                data_entry = to_offset(resource_rva + offset)
                data_rva, size = struct.unpack_from("<II", data, data_entry)
                start = to_offset(data_rva)
                result.setdefault(path[0], {})[path[1] if len(path) > 1 else 0] = data[start:start + size]

    walk(root, 0, [])
    return result


def png_pixels(path: Path) -> tuple:
    """用标准库解码 PNG，返回 (宽, 高, [(r,g,b), ...])。仅支持本项目生成的 8 位 RGB/RGBA 非隔行 PNG。"""
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("不是 PNG：%s" % path)
    pos = 8
    width = height = None
    bit_depth = color_type = None
    idat = bytearray()
    while pos < len(data):
        (length,) = struct.unpack_from(">I", data, pos)
        chunk_type = data[pos + 4:pos + 8]
        payload = data[pos + 8:pos + 8 + length]
        if chunk_type == b"IHDR":
            width, height, bit_depth, color_type, _comp, _filt, interlace = struct.unpack_from(">IIBBBBB", payload, 0)
            if bit_depth != 8 or color_type not in (2, 6) or interlace != 0:
                raise ValueError("不支持的 PNG 格式：bit=%d color=%d interlace=%d" % (bit_depth, color_type, interlace))
        elif chunk_type == b"IDAT":
            idat += payload
        elif chunk_type == b"IEND":
            break
        pos += 12 + length
    channels = 3 if color_type == 2 else 4
    stride = width * channels
    raw = zlib.decompress(bytes(idat))
    rows = []
    previous = bytearray(stride)
    offset = 0
    for _ in range(height):
        filter_type = raw[offset]
        line = bytearray(raw[offset + 1:offset + 1 + stride])
        offset += 1 + stride
        for index in range(stride):
            left = line[index - channels] if index >= channels else 0
            up = previous[index]
            up_left = previous[index - channels] if index >= channels else 0
            if filter_type == 1:
                line[index] = (line[index] + left) & 0xFF
            elif filter_type == 2:
                line[index] = (line[index] + up) & 0xFF
            elif filter_type == 3:
                line[index] = (line[index] + ((left + up) // 2)) & 0xFF
            elif filter_type == 4:
                p = left + up - up_left
                pa, pb, pc = abs(p - left), abs(p - up), abs(p - up_left)
                predictor = left if (pa <= pb and pa <= pc) else (up if pb <= pc else up_left)
                line[index] = (line[index] + predictor) & 0xFF
            elif filter_type != 0:
                raise ValueError("未知 PNG 过滤器 %d" % filter_type)
        rows.extend((line[i], line[i + 1], line[i + 2]) for i in range(0, stride, channels))
        previous = line
    return width, height, rows


def group_entries(groups: dict) -> list:
    """解析组图标载荷：GRPICONDIR(6) + N * GRPICONDIRENTRY(14)，返回 [(名称, [(宽,高,资源ID)...])]。"""
    declared = []
    for name, payload in sorted(groups.items()):
        count = struct.unpack_from("<H", payload, 4)[0]
        entries = []
        for index in range(count):
            base = 6 + 14 * index
            width, height, _c, _r, _p, _b, _bytes, resource_id = struct.unpack_from("<BBBBHHIH", payload, base)
            entries.append(((width or 256), (height or 256), resource_id))
        declared.append((name, entries))
        print("[check_exe_icon] 组图标 #%s：%d 条目 -> %s"
              % (name, count, [(w, h) for (w, h, _rid) in entries]))
    return declared


def verify_pe_icon(exe: Path, check) -> None:
    """校验 PE 资源节中的图标：类型/数量/尺寸集合/逐帧字节一致。"""
    resources = pe_resources(exe.read_bytes())
    icons = resources.get(RT_ICON, {})
    groups = resources.get(RT_GROUP_ICON, {})
    check(bool(resources), "存在 PE 资源目录", "类型: %s" % sorted(resources))
    check(bool(groups), "存在 RT_GROUP_ICON(14)", "条目 %d 个" % len(groups))
    check(bool(icons), "存在 RT_ICON(3)", "条目 %d 个" % len(icons))

    declared = group_entries(groups)
    sizes = tuple(sorted(w for (_n, entries) in declared for (w, _h, _r) in entries))
    check(sizes == EXPECTED_SIZES, "组图标声明的尺寸集合", "实测 %s" % (list(sizes),))
    check(len(icons) == len(EXPECTED_SIZES), "RT_ICON 条目数",
          "实测 %d，期望 %d" % (len(icons), len(EXPECTED_SIZES)))

    frames = read_ico_frames(ICO)
    references = {}
    for (_name, entries) in declared:
        for (width, height, resource_id) in entries:
            references[(width, height)] = icons.get(resource_id)
    for size in EXPECTED_SIZES:
        reference = frames.get((size, size))
        payload = references.get((size, size))
        if reference is None or payload is None:
            check(False, "尺寸 %d 的帧" % size, "源帧或 exe 内嵌帧缺失")
            continue
        check(payload == reference, "尺寸 %d 帧与 app.ico 逐字节一致" % size,
              "%s (%d 字节)" % ("一致" if payload == reference else "不一致", len(payload)))


def verify_shell_icon(shell_icon: Path, check) -> None:
    """校验 Windows 外壳关联图标（外部生成的 32x32 PNG）与 app_32.png 逐像素一致。"""
    reference_png = RES_DIR / "app_32.png"
    if not shell_icon.is_file():
        check(False, "Windows 外壳关联图标截图", "缺少 %s" % shell_icon)
        return
    if not reference_png.is_file():
        check(False, "Windows 外壳关联图标比对", "缺少 %s" % reference_png)
        return
    width, height, shell_pixels = png_pixels(shell_icon)
    ref_width, ref_height, ref_pixels = png_pixels(reference_png)
    same_geometry = (width, height) == (ref_width, ref_height) == (32, 32)
    aligned = same_geometry and len(shell_pixels) == len(ref_pixels)
    differing = sum(1 for a, b in zip(shell_pixels, ref_pixels) if a != b) if aligned else -1
    check(aligned and differing == 0, "Windows 外壳关联图标与 app_32.png 逐像素一致",
          "外壳图标 %dx%d，差异像素 %s" % (width, height, differing))


def main() -> int:
    args = sys.argv[1:]
    shell_icon = None
    if "--shell-icon" in args:
        index = args.index("--shell-icon")
        shell_icon = Path(args[index + 1])
        del args[index:index + 2]
    exe = Path(args[0]) if args else DEFAULT_EXE
    failures = []

    def check(ok: bool, label: str, detail: str = "") -> None:
        print("[check_exe_icon] %s %s%s" % ("OK  " if ok else "FAIL", label,
                                            (" — " + detail) if detail else ""))
        if not ok:
            failures.append(label)

    print("[check_exe_icon] exe=%s" % exe)
    print("[check_exe_icon] ico=%s" % ICO)
    if not exe.is_file():
        sys.stderr.write("[check_exe_icon] 缺少编译产物：%s\n" % exe)
        return 1
    if not ICO.is_file():
        sys.stderr.write("[check_exe_icon] 缺少图标源：%s\n" % ICO)
        return 1

    verify_pe_icon(exe, check)
    if shell_icon is not None:
        verify_shell_icon(shell_icon, check)

    if failures:
        sys.stderr.write("[check_exe_icon] FAILED（%d 项）：%s\n" % (len(failures), "; ".join(failures)))
        return 1
    print("[check_exe_icon] PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
