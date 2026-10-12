#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_app_icon.py -- 生成 PsigMatcher 的程序图标资源（可复现）。

产物（均写入 gui/res/）：
  - app.ico            多尺寸 Windows 图标资源（PE 资源用；rc.exe 编译 app.rc 引用它）
  - app_<N>.png        N = 16/24/32/48/64/128/256，供多尺寸 QIcon 使用（:/app_<N>.png）

设计（沿用既有 app.png 的视觉识别，两色 + 对角带）：
  深蓝底 #2A4A7A，亮蓝对角带 #6FA8E0；
  取像素中心归一化坐标 u=(x+0.5)/N、v=(y+0.5)/N，当 |u-v| <= 5/32 时该像素为亮带。
  该规则在 N=32 时与仓库既有 gui/res/app.png（v0.3.2 及其之前的单尺寸窗口图标）逐像素一致；
  自 v0.3.3 起该单尺寸文件由本脚本生成的 app_32.png 取代。

用法：
  python3 gui/script/gen_app_icon.py            # 生成全部产物
  python3 gui/script/gen_app_icon.py --check    # 只校验：生成结果与磁盘上的既有文件是否一致

依赖：Python 3 + Pillow（仅开发期使用，不进入程序构建/运行期依赖）。
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

try:
    from PIL import Image  # type: ignore
except ImportError:  # pragma: no cover - 环境错误分支
    sys.stderr.write(
        "[gen_app_icon] 缺少 Pillow：请先安装（例如 python3 -m pip install pillow）。\n"
        "[gen_app_icon] 本脚本不会生成半成品产物。\n"
    )
    raise SystemExit(2)

SIZES = (16, 24, 32, 48, 64, 128, 256)
PNG_SIZES = (128, 256)  # 大尺寸用 PNG 压缩条目（ICO 自 Vista 起支持）
BAND_HALF_WIDTH = 5.0 / 32.0  # 对角带半宽（归一化）
BACKGROUND = (0x2A, 0x4A, 0x7A)  # 深蓝底
BAND_COLOR = (0x6F, 0xA8, 0xE0)  # 亮蓝对角带
RES_DIR = Path(__file__).resolve().parent.parent / "res"


def render(size: int) -> "Image.Image":
    """按几何规则渲染 size x size 图标（不透明 RGB，逐像素判定，确定性）。"""
    img = Image.new("RGB", (size, size), BACKGROUND)
    pixels = img.load()
    for y in range(size):
        v = (y + 0.5) / size
        for x in range(size):
            u = (x + 0.5) / size
            if abs(u - v) <= BAND_HALF_WIDTH:
                pixels[x, y] = BAND_COLOR
    return img


def encode_png(img: "Image.Image") -> bytes:
    import io

    buf = io.BytesIO()
    img.save(buf, format="PNG", optimize=True)
    return buf.getvalue()


def bmp_entry_payload(img: "Image.Image") -> bytes:
    """32bpp BMP 条目：BITMAPINFOHEADER + 自下而上的 BGRA 像素 + AND 掩码（全不透明）。"""
    size = img.width
    pixels = img.load()
    header = struct.pack(
        "<IiiHHIIiiII",
        40,           # biSize
        size,         # biWidth
        size * 2,     # biHeight（= XOR 图 + AND 掩码）
        1,            # biPlanes
        32,           # biBitCount
        0,            # biCompression = BI_RGB
        size * size * 4,  # biSizeImage
        0, 0, 0, 0,   # 分辨率与调色板
    )
    body = bytearray()
    for y in range(size - 1, -1, -1):  # 自下而上
        for x in range(size):
            r, g, b = pixels[x, y]
            body += bytes((b, g, r, 0xFF))
    mask_row = ((size + 31) // 32) * 4  # 1bpp，按 4 字节对齐
    body += bytes(mask_row * size)      # 全 0 = 全部不透明（与 alpha=0xFF 一致）
    return header + bytes(body)


def build_ico() -> bytes:
    """组装 ICO 容器（小尺寸 BMP 条目 + 大尺寸 PNG 条目）。"""
    frames = []
    for size in SIZES:
        img = render(size)
        payload = encode_png(img) if size in PNG_SIZES else bmp_entry_payload(img)
        frames.append((size, payload))

    out = bytearray(struct.pack("<HHH", 0, 1, len(frames)))  # ICONDIR
    offset = 6 + 16 * len(frames)
    for size, payload in frames:
        dimension = 0 if size >= 256 else size  # 256 在目录项中以 0 表示
        out += struct.pack(
            "<BBBBHHII",
            dimension, dimension, 0, 0, 1, 32, len(payload), offset,
        )
        offset += len(payload)
    for _, payload in frames:
        out += payload
    return bytes(out)


def targets() -> dict:
    """待写出的文件：相对 gui/res 的路径 -> 字节内容。"""
    files = {"app.ico": build_ico()}
    for size in SIZES:
        files["app_%d.png" % size] = encode_png(render(size))
    return files


def main() -> int:
    parser = argparse.ArgumentParser(description="生成 PsigMatcher 程序图标资源")
    parser.add_argument("--check", action="store_true",
                        help="只校验磁盘上的生成物与当前脚本输出是否一致（不写盘）")
    args = parser.parse_args()

    files = targets()
    failures = []
    for name, data in sorted(files.items()):
        path = RES_DIR / name
        if args.check:
            if not path.is_file():
                failures.append("%s: 缺失" % name)
            elif path.read_bytes() != data:
                failures.append("%s: 内容与脚本输出不一致（%d 字节 -> %d 字节）"
                                % (name, path.stat().st_size, len(data)))
            else:
                print("[gen_app_icon] OK   %s (%d 字节)" % (name, len(data)))
            continue
        path.write_bytes(data)
        print("[gen_app_icon] 写入 %s (%d 字节)" % (name, len(data)))

    if args.check:
        if failures:
            for item in failures:
                sys.stderr.write("[gen_app_icon] FAIL %s\n" % item)
            return 1
        print("[gen_app_icon] 全部生成物与脚本输出一致")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
