# gui/script 脚本目录

按生命周期分类存放，避免脚本越积越乱。新建脚本前必须先归入以下四类之一，并同步本 README。

## 核心（长期保留，根目录）

每次构建/回归必用的长期脚本：

| 脚本 | 用途 |
| --- | --- |
| `build_debug.bat` | Windows Debug 编译（共享 Qt /MDd） |
| `build_release.bat` | Windows Release 编译（静态 Qt /MT） |
| `build_debug_wsl.sh` | WSL 入口 → cmd.exe → build_debug.bat |
| `build_release_wsl.sh` | WSL 入口 → cmd.exe → build_release.bat |

## 验证（`verify/`）

构建后可复用的可验证手段，长期保留。例如 dumpbin 依赖/指令检查（Release 静态链接策略）。

## 排查（`diagnose/`）

定位开发期问题时临时创建的工具（如读模态框、查链接、后台重建）。问题解决并验证后**必须删除**，不留存。

## 一次性工具（`tools/`）

补丁/改写流程文档等一次性脚本。内容并入文档后删除或归档。

## 约定

- 脚本内部引用同级或上级脚本优先用绝对路径（或相对 `gui/` 的路径），目录移动不破坏引用。
- 无法归类的临时脚本一律视为排查类，用完即删。
