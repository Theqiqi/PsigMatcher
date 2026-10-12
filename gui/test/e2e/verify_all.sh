#!/usr/bin/env bash
# verify_all.sh -- PsigMatcher E2E 编排脚本
# 顺序固定：清理旧进程/旧产物 → 启动程序并驱动（drive.ps1）→ 关闭程序 → 像素断言（analyze.py）
#          → 图标资源校验（源侧/PE 产物侧/Windows 外壳关联图标；v0.3.3 追加，非像素断言）
# 用法: bash gui/test/e2e/verify_all.sh [Debug|Release]   默认 Release
set -uo pipefail

CFG="${1:-Release}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GUI_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
OUT_DIR="$GUI_DIR/out/verify"
EXE="$GUI_DIR/out/bin/$CFG/PsigMatcher.exe"

echo "[verify_all] config=$CFG"
echo "[verify_all] exe=$EXE"
if [ ! -f "$EXE" ]; then
  echo "[verify_all] 缺少编译产物，请先编译（build_${CFG,,}.bat）"
  exit 1
fi

# 1) 清理旧进程
taskkill.exe /f /im PsigMatcher.exe 2>/dev/null || true
taskkill.exe /f /im psig_target.exe 2>/dev/null || true

# 2) 清理旧产物
rm -f "$OUT_DIR"/shot_*.png "$OUT_DIR"/exit_code.txt "$OUT_DIR"/exe_associated_icon.png "$OUT_DIR"/*.log 2>/dev/null || true
mkdir -p "$OUT_DIR"

# 3) 启动 + 驱动 + 关闭
DRIVE_PS="$(wslpath -w "$SCRIPT_DIR/drive.ps1")"
EXE_WIN="$(wslpath -w "$EXE")"
OUT_WIN="$(wslpath -w "$OUT_DIR")"
cmd.exe /c "powershell -NoProfile -ExecutionPolicy Bypass -File $DRIVE_PS -Exe $EXE_WIN -OutDir $OUT_WIN"
DRIVE_RC=$?
echo "[verify_all] drive.ps1 rc=$DRIVE_RC"

# 4) 断言
python3 "$SCRIPT_DIR/analyze.py" "$OUT_DIR"
AN_RC=$?
echo "[verify_all] analyze.py rc=$AN_RC"

# 5) 图标资源校验（C-01；不依赖 GUI 运行，故在像素断言后追加）
VERIFY_DIR="$GUI_DIR/script/verify"
python3 "$VERIFY_DIR/check_app_icon.py"
APP_ICON_RC=$?
echo "[verify_all] check_app_icon.py rc=$APP_ICON_RC"

EXTRACT_PS="$(wslpath -w "$VERIFY_DIR/extract_exe_icon.ps1")"
cmd.exe /c "powershell -NoProfile -ExecutionPolicy Bypass -File $EXTRACT_PS -Exe $EXE_WIN -OutDir $OUT_WIN"
EXTRACT_RC=$?
echo "[verify_all] extract_exe_icon.ps1 rc=$EXTRACT_RC"

python3 "$VERIFY_DIR/check_exe_icon.py" "$EXE" --shell-icon "$OUT_DIR/exe_associated_icon.png"
EXE_ICON_RC=$?
echo "[verify_all] check_exe_icon.py rc=$EXE_ICON_RC"

if [ "$DRIVE_RC" -ne 0 ] || [ "$AN_RC" -ne 0 ] \
   || [ "$APP_ICON_RC" -ne 0 ] || [ "$EXTRACT_RC" -ne 0 ] || [ "$EXE_ICON_RC" -ne 0 ]; then
  echo "[verify_all] FAILED"
  exit 1
fi
echo "[verify_all] OK"
exit 0
