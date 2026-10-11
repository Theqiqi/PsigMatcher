#!/usr/bin/env python3
"""analyze.py -- psig_gui E2E 像素断言脚本

读取 drive.ps1 产出的 out/verify/shot_xx.png 截图，基于颜色/区域统计断言每个场景的
真实画面状态，输出通过/失败汇总并以退出码返回。

用法: python3 analyze.py [verify_dir]
默认 verify_dir = <repo>/gui/out/verify
"""
import os
import sys

from PIL import Image, ImageChops, ImageStat

DEFAULT_VERIFY = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                              "..", "..", "out", "verify")


def mean_luma(img):
    """平均亮度（0..255）。"""
    stat = ImageStat.Stat(img.convert("L"))
    return stat.mean[0]


def ui_ratio(img, bg_tol=18):
    """非背景像素占比：以出现最多的颜色为背景，统计与其差异超阈值的像素比例。"""
    small = img.convert("RGB").resize((320, 200))
    colors = small.getcolors(maxcolors=320 * 200)
    if not colors:
        return 0.0
    colors.sort(reverse=True)
    bg = colors[0][1]
    total = small.width * small.height
    diff = 0
    px = small.load()
    for y in range(small.height):
        for x in range(small.width):
            r, g, b = px[x, y]
            if abs(r - bg[0]) + abs(g - bg[1]) + abs(b - bg[2]) > bg_tol:
                diff += 1
    return diff / total


def diff_ratio(a, b):
    """两张图差异像素比例（缩放到同尺寸后逐像素比较）。"""
    size = (480, 300)
    ia = a.convert("RGB").resize(size)
    ib = b.convert("RGB").resize(size)
    ch = ImageChops.difference(ia, ib).convert("L")
    px = ch.load()
    total = size[0] * size[1]
    changed = 0
    for y in range(size[1]):
        for x in range(size[0]):
            if px[x, y] > 12:
                changed += 1
    return changed / total


class Report:
    def __init__(self):
        self.passed = 0
        self.failed = 0

    def check(self, case, ok, detail):
        tag = "PASS" if ok else "FAIL"
        if ok:
            self.passed += 1
        else:
            self.failed += 1
        print(f"[{tag}] {case}: {detail}")


def load(verify_dir, name):
    path = os.path.join(verify_dir, name)
    if not os.path.exists(path):
        return None
    return Image.open(path)


def main():
    verify_dir = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_VERIFY
    verify_dir = os.path.abspath(verify_dir)
    print(f"[analyze] verify_dir = {verify_dir}")
    rep = Report()

    shots = {
        "start": load(verify_dir, "shot_01_start.png"),
        "process_refreshed": load(verify_dir, "shot_02_process_refreshed.png"),
        "signature": load(verify_dir, "shot_03_signature.png"),
        "addrow": load(verify_dir, "shot_04_addrow.png"),
        "result": load(verify_dir, "shot_05_result.png"),
        "about": load(verify_dir, "shot_06_about.png"),
    }

    # E2E-01 启动主界面（v0.2.0：启动即进程页）
    s = shots["start"]
    if s is None:
        rep.check("E2E-01 启动主界面", False, "缺少 shot_01_start.png")
    else:
        luma = mean_luma(s)
        ratio = ui_ratio(s)
        big = s.width >= 600 and s.height >= 400
        rep.check("E2E-01 启动主界面",
                  (luma > 30) and (ratio > 0.02) and big,
                  f"size={s.width}x{s.height} luma={luma:.1f} ui_ratio={ratio:.3f}")

    # E2E-02 刷新进程（进程表被填充，画面变化）
    if shots["process_refreshed"] and shots["start"]:
        d = diff_ratio(shots["process_refreshed"], shots["start"])
        rep.check("E2E-02 刷新进程填充列表", d > 0.005, f"diff={d:.5f}")
    else:
        rep.check("E2E-02 刷新进程填充列表", False, "缺少截图")

    # E2E-03 页面导航（不同页面画面不同；启动页=进程页）
    if shots["signature"] and shots["start"]:
        d = diff_ratio(shots["signature"], shots["start"])
        rep.check("E2E-03a 特征码页≠进程页", d > 0.004, f"diff={d:.4f}")
    else:
        rep.check("E2E-03a 特征码页≠进程页", False, "缺少页面截图")
    if shots["result"] and shots["signature"]:
        d = diff_ratio(shots["result"], shots["signature"])
        rep.check("E2E-03b 结果页≠特征码页", d > 0.004, f"diff={d:.4f}")
    else:
        rep.check("E2E-03b 结果页≠特征码页", False, "缺少页面截图")

    # E2E-04 新增特征码行（特征码页内容变化）
    if shots["addrow"] and shots["signature"]:
        d = diff_ratio(shots["addrow"], shots["signature"])
        rep.check("E2E-04 新增行改变特征码页", d > 0.001, f"diff={d:.5f}")
    else:
        rep.check("E2E-04 新增行改变特征码页", False, "缺少截图")

    # E2E-06 关于弹窗（模态弹窗覆盖主窗口，画面变化）
    if shots["about"] and shots["result"]:
        d = diff_ratio(shots["about"], shots["result"])
        rep.check("E2E-06 关于弹窗覆盖主窗口", d > 0.004, f"diff={d:.5f}")
    else:
        rep.check("E2E-06 关于弹窗覆盖主窗口", False, "缺少截图")

    # E2E-07 关闭程序退出码为 0
    code_path = os.path.join(verify_dir, "exit_code.txt")
    if os.path.exists(code_path):
        code = open(code_path).read().strip()
        rep.check("E2E-07 正常关闭退出码0", code == "0", f"exit_code={code}")
    else:
        rep.check("E2E-07 正常关闭退出码0", False, "缺少 exit_code.txt")

    print(f"\n[analyze] 汇总: {rep.passed} passed, {rep.failed} failed")
    return 0 if rep.failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
