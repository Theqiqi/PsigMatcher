#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""check_app_icon.py -- 校验「图标资源」的源侧一致性（生成物 ↔ 生成脚本 ↔ .qrc ↔ main.cpp）。

校验项（任一不满足即非零退出码）：
  1. gui/res 中 7 个 app_<N>.png 与 app.ico 均存在；
  2. 生成脚本可重建全部生成物（调用 gen_app_icon.py --check，逐字节一致）；
  3. app.ico 的帧尺寸集合恰为 {16,24,32,48,64,128,256}；
  4. app.ico 每一帧的像素与对应 app_<N>.png 逐像素一致；
  5. gui/res/resources.qrc 收录的 app_<N>.png 集合与生成脚本的尺寸集合一致；
  6. gui/src/main.cpp 的 kAppIconSizes 与生成脚本的尺寸集合一致（防三处漂移）。

用法：python3 gui/script/verify/check_app_icon.py
依赖：Python 3 + Pillow（开发期依赖，不进入程序构建/运行期依赖）。
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:  # pragma: no cover - 环境错误分支
    sys.stderr.write("[check_app_icon] 缺少 Pillow：请先安装（python3 -m pip install pillow）。\n")
    raise SystemExit(2)

SCRIPT_DIR = Path(__file__).resolve().parent
GUI_DIR = SCRIPT_DIR.parent.parent
RES_DIR = GUI_DIR / "res"
GEN_SCRIPT = GUI_DIR / "script" / "gen_app_icon.py"
QRC = RES_DIR / "resources.qrc"
MAIN_CPP = GUI_DIR / "src" / "main.cpp"

EXPECTED_SIZES = (16, 24, 32, 48, 64, 128, 256)


def pixels(image: "Image.Image") -> list:
    """取像素序列（Pillow 14 起 getdata 被 get_flattened_data 取代，两者兼容）。"""
    getter = getattr(image, "get_flattened_data", None)
    return list(getter() if getter else image.getdata())


def generator_sizes() -> tuple:
    """读取生成脚本声明的尺寸集合（解析 SIZES 常量）。"""
    text = GEN_SCRIPT.read_text(encoding="utf-8")
    match = re.search(r"^SIZES\s*=\s*\(([^)]*)\)", text, re.MULTILINE)
    if not match:
        raise SystemExit("[check_app_icon] 无法解析 gen_app_icon.py 的 SIZES 常量")
    return tuple(int(v.strip()) for v in match.group(1).split(",") if v.strip())


def qrc_sizes() -> tuple:
    text = QRC.read_text(encoding="utf-8")
    return tuple(sorted(int(m) for m in re.findall(r"<file>app_(\d+)\.png</file>", text)))


def main_cpp_sizes() -> tuple:
    text = MAIN_CPP.read_text(encoding="utf-8")
    match = re.search(r"kAppIconSizes\[\]\s*=\s*\{([^}]*)\}", text)
    if not match:
        raise SystemExit("[check_app_icon] 无法解析 main.cpp 的 kAppIconSizes 常量")
    return tuple(int(v.strip()) for v in match.group(1).split(",") if v.strip())


def main() -> int:
    failures = []

    def check(ok: bool, label: str, detail: str = "") -> None:
        print("[check_app_icon] %s %s%s" % ("OK  " if ok else "FAIL", label,
                                            (" — " + detail) if detail else ""))
        if not ok:
            failures.append(label)

    sizes = generator_sizes()
    check(tuple(sorted(sizes)) == EXPECTED_SIZES, "生成脚本尺寸集合",
          "实测 %s" % (list(sizes),))

    # 1 + 2：生成物存在且可被脚本逐字节重建
    ico_path = RES_DIR / "app.ico"
    check(ico_path.is_file(), "app.ico 存在", str(ico_path))
    for size in sizes:
        png = RES_DIR / ("app_%d.png" % size)
        check(png.is_file(), "app_%d.png 存在" % size, str(png))

    proc = subprocess.run([sys.executable, str(GEN_SCRIPT), "--check"],
                          capture_output=True, text=True)
    check(proc.returncode == 0, "生成物可被 gen_app_icon.py --check 逐字节重建",
          "rc=%d" % proc.returncode)
    if proc.returncode != 0:
        sys.stderr.write(proc.stdout + proc.stderr)

    # 3 + 4：ICO 帧尺寸集合与逐帧像素一致
    if ico_path.is_file():
        with Image.open(ico_path) as ico:
            ico_sizes = tuple(sorted(w for (w, _h) in ico.ico.sizes()))
            check(ico_sizes == EXPECTED_SIZES, "app.ico 帧尺寸集合",
                  "实测 %s" % (list(ico_sizes),))
            for size in EXPECTED_SIZES:
                frame = ico.ico.getimage((size, size)).convert("RGB")
                with Image.open(RES_DIR / ("app_%d.png" % size)) as png:
                    reference = png.convert("RGB")
                same = frame.size == reference.size and pixels(frame) == pixels(reference)
                check(same, "app.ico 第 %d 帧与 app_%d.png 逐像素一致" % (size, size),
                      "" if same else "尺寸/像素不一致")

    # 5 + 6：三处尺寸集合同步（脚本 / .qrc / main.cpp）
    check(qrc_sizes() == EXPECTED_SIZES, "resources.qrc 收录的尺寸集合",
          "实测 %s" % (list(qrc_sizes()),))
    check(main_cpp_sizes() == EXPECTED_SIZES, "main.cpp kAppIconSizes 尺寸集合",
          "实测 %s" % (list(main_cpp_sizes()),))

    if failures:
        sys.stderr.write("[check_app_icon] FAILED（%d 项）：%s\n" % (len(failures), "; ".join(failures)))
        return 1
    print("[check_app_icon] PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
