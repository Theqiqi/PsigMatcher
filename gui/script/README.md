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
| `run_debug_tests.bat` | 运行 Debug 单元/集成测试（ctest，Qt bin 入 PATH） |
| `run_release_tests.bat` | 运行 Release 单元/集成测试（ctest，静态无 DLL 依赖） |
| `run_debug_tests_wsl.sh` | WSL 入口 → cmd.exe → run_debug_tests.bat |
| `run_release_tests_wsl.sh` | WSL 入口 → cmd.exe → run_release_tests.bat |

## 验证（`verify/`）

构建后可复用的可验证手段，长期保留：

| 脚本 | 用途 |
| --- | --- |
| `dump_dependents.bat` | `dumpbin /dependents` 抽查生产 exe 的 DLL 依赖（校验运行库链接策略：Release 无 VCRUNTIME/MSVCP/Qt DLL，Debug 依赖 *D.dll） |
| `dump_directives.bat` | `dumpbin /directives` 抽查 `psig.lib` 的 `/DEFAULTLIB`（Release=LIBCMT/libcpmt，Debug=MSVCRTD/msvcprtd） |

## 排查（`diagnose/`）

定位开发期问题时临时创建的工具。问题解决并验证后**必须删除**，不留存。当前为空。

## 一次性工具（`tools/`）

补丁/改写流程文档等一次性脚本。内容并入文档后删除或归档。当前为空。

## 约定

- 脚本内部引用同级或上级脚本优先用绝对路径（或相对 `gui/` 的路径），目录移动不破坏引用。
- 无法归类的临时脚本一律视为排查类，用完即删。
- E2E 编排脚本（`test/e2e/verify_all.sh` 等）归属测试体系，不放入 `script/`。
