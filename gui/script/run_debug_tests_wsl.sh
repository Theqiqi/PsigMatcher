#!/usr/bin/env bash
# WSL Debug 测试脚本：经 cmd.exe 调用 run_debug_tests.bat
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BAT_WIN="$(wslpath -w "$SCRIPT_DIR/run_debug_tests.bat")"
cmd.exe /c "$BAT_WIN"
