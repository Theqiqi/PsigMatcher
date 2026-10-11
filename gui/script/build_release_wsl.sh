#!/usr/bin/env bash
# WSL Release 编译脚本：经 cmd.exe 调用 build_release.bat（WSL -> cmd.exe -> .bat -> MSVC）
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BAT_WIN="$(wslpath -w "$SCRIPT_DIR/build_release.bat")"
cmd.exe /c "$BAT_WIN"
