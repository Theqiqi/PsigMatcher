#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""check_exe_icon.py -- 校验编译产物 exe 的 PE 图标资源（产物侧，与源侧 check_app_icon.py 对应）。

校验项（任一不满足即非零退出码）：
  1. 目标文件是 PE 且存在资源目录（IMAGE_DIRECTORY_ENTRY_RESOURCE）；
  2. 资源目录中存在 RT_GROUP_ICON (14) 与 RT_ICON (3)；
  3. 组图标声明的条目数 == 7，且尺寸集合 == {16,24,32,48,64,128,256}；
  4. 每个 RT_ICON 载荷与 gui/res/app.ico 中对应尺寸帧的字节一致（逐字节比对）。

用法：python3 gui/script/verify/check_exe_icon.py [exe 路径]
      默认 gui/out/bin/Release/PsigMatcher.exe
依赖：仅标准库（不依赖 Pillow）。
"""

from __future__ import annotations

import struct
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
GUI_DIR = SCRIPT_DIR.parent.parent
DEFAULT_EXE = GUI_DIR / "out" / "bin" / "Release" / "PsigMatcher.exe"
ICO = GUI_DIR / "res" / "app.ico"

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
        named, id_count = struct.unpack_from("<HH", data, directory_offset + 12)
        for index in range(named + id_count):
            entry = directory_offset + 16 + 8 * index
            name_id, offset = struct.unpack_from("<II", data, entry)
            if offset & 0x80000000:
                continue  # 具名（字符串）条目，图标资源不使用
            if depth < 2:
                walk(to_offset(resource_rva + (offset & 0x7FFFFFFF)), depth + 1, path + [name_id])
            else:
                data_entry = to_offset(resource_rva + offset)
                data_rva, size = struct.unpack_from("<II", data, data_entry)
                start = to_offset(data_rva)
                result.setdefault(path[0], {})[path[1] if len(path) > 1 else 0] = data[start:start + size]

    walk(root, 0, [])
    return result


def main() -> int:
    exe = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_EXE
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

    resources = pe_resources(exe.read_bytes())
    check(bool(resources), "存在 PE 资源目录", "类型: %s" % sorted(resources))
    icons = resources.get(RT_ICON, {})
    groups = resources.get(RT_GROUP_ICON, {})
    check(bool(groups), "存在 RT_GROUP_ICON(14)", "条目 %d 个" % len(groups))
    check(bool(icons), "存在 RT_ICON(3)", "条目 %d 个" % len(icons))

    # 组图标结构：GRPICONDIR(6) + N * GRPICONDIRENTRY(14)
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

    sizes = tuple(sorted(w for (_n, entries) in declared for (w, _h, _r) in entries))
    check(sizes == EXPECTED_SIZES, "组图标声明的尺寸集合", "实测 %s" % (list(sizes),))
    check(len(icons) == len(EXPECTED_SIZES), "RT_ICON 条目数",
          "实测 %d，期望 %d" % (len(icons), len(EXPECTED_SIZES)))

    # 逐字节比对 exe 内嵌帧与源 app.ico 对应帧
    frames = read_ico_frames(ICO)
    references = {}
    for (_name, entries) in declared:
        for (width, height, resource_id) in entries:
            references[(width, height)] = (resource_id, icons.get(resource_id))
    for size in EXPECTED_SIZES:
        reference = frames.get((size, size))
        resource_id, payload = references.get((size, size), (None, None))
        if reference is None or payload is None:
            check(False, "尺寸 %d 的帧" % size, "源帧或 exe 内嵌帧缺失")
            continue
        check(payload == reference, "尺寸 %d 帧与 app.ico 逐字节一致" % size,
              "%s (%d 字节)" % ("一致" if payload == reference else "不一致", len(payload)))

    if failures:
        sys.stderr.write("[check_exe_icon] FAILED（%d 项）：%s\n" % (len(failures), "; ".join(failures)))
        return 1
    print("[check_exe_icon] PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
