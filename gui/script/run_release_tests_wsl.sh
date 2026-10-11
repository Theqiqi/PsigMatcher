#!/usr/bin/env bash
# WSL Release 测试脚本：经 cmd.exe 调用 run_release_tests.bat
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BAT_WIN="$(wslpath -w "$SCRIPT_DIR/run_release_tests.bat")"
cmd.exe /c "$BAT_WIN"
