# psig — Windows 进程批量特征码匹配器

以**表格**维护特征码并对其**批量扫描 Windows 进程内存**的桌面工具：输入是特征码表，输出是匹配结果表，两者都可在软件内查看、编辑、导入、导出。

- 仓库：<https://github.com/Theqiqi/PsigMatcher>
- 版本：**v0.3.2**（tag `v0.3.2`，设计与代码同号归档）
- 形态：Windows 桌面 Qt6 GUI 单体程序（`PsigMatcher.exe`）+ 业务静态库（`psig.lib`）
- 语言/工具链：C++20 / MSVC（`cl.exe`）+ Ninja + CMake ≥ 3.20 + vcpkg manifest

## 目录结构

```
env/                        项目根目录（根目录只放源码子目录与配置，不放构建产物）
├─ gui/                     带 GUI 的单体程序（MVVM 4 层）
│  ├─ CMakeLists.txt        project(PsigMatcher)：公共选项 + 子目录入口
│  ├─ CMakePresets.json     debug（共享 Qt /MDd）与 release（静态 Qt /MT）预设
│  ├─ vcpkg.json            gtest（Qt6 独立安装，不走 vcpkg）
│  ├─ src/
│  │  ├─ main.cpp           组装：创建 View → 创建 ViewModel → install(view) → show → exec
│  │  ├─ view/              只画 UI（无 signals / 无 connect / 无事件逻辑）
│  │  ├─ model/             纯数据（QAbstractTableModel，无业务逻辑）
│  │  └─ viewmodel/         ViewModel（持 View*/Model*，集中 connect，调静态库）
│  ├─ res/                  resources.qrc、style.qss、app.png、i18n/*.ts（中文源语言 + 英文）
│  ├─ script/               构建/测试/验证脚本（见 script/README.md 的四类管理）
│  ├─ test/                 unit | integration | target | e2e
│  └─ out/                  构建产物（out/bin/<配置>/PsigMatcher.exe，out/lib、out/build，已 gitignore）
├─ psig/                    业务静态库（唯一对外契约：include/psig.h）
│  ├─ include/psig.h        公共 API（不依赖 Qt、不抛异常、线程安全）
│  ├─ src/data → algorithm → api    自底向上分层；src/atomic 为 Win32/文件/SQLite 原子能力
│  ├─ third_party/sqlite3/  内置 SQLite（`.db` 读写）
│  ├─ test/                 unit（psig_unit）| integration（psig_integration）| target（psig_target）
│  ├─ script/               build_debug/release（.bat + _wsl.sh）
│  └─ out/lib/<配置>/psig.lib
├─ design/                  设计文档，按版本目录归档（v0.1.0、v0.2.x、v0.3.x、v0.3.2=当前）
├─ .flow/                   流程规范（新建程序 / 修改添加功能 / 静态库）
└─ .idea/                   每个版本的需求输入（idea.md、v0.3.2.md …）
```

## 功能特性

**输入侧（特征码表）**
- 表格化编辑，列可自由增删（内置 `名称/特征码/备注` + GUI 内置 `进程` 列）
- 特征码支持通配符模糊匹配：`48 8B ?? 05`、`488B??05`、`?8 8B A?` 等写法
- 导入：特征码表（CSV / SQLite `.db`）；导出：特征码表（CSV / `.db`，`.db` 携带目标进程名）
- **未保存改动真实跟踪**：单元格编辑、增行/删行、列变更都计入；导出/导入成功即视为已保存；持久状态区显示 `*`
- 有未保存改动时：导入仅做**覆盖确认**（继续导入 / 取消）；关闭程序或「重新选择进程」提示 **保存 / 不保存 / 取消**

**输出侧（匹配结果表）**
- 多进程 × 多特征码批量扫描，结果按行汇总（特征码名、进程名/PID、地址、模块、命中字节、命中时间、自定义列）
- 结果页可删除行、编辑末列自定义值、批量填充、按需选择导出列、维护表级模板文本
- 导出：CSV / TXT（GUI 入口）；静态库另支持结果 `.db`

**其它**
- 中 / 英界面切换（含导出表头与 TXT 摘要随语言）；进程列表筛选 + 排序 + 权限图标
- 扫描进度条、历史日志 dock（按级别着色）、统一白底 + 颜色点缀的界面风格

## 环境要求

| 项 | 要求 |
| --- | --- |
| 操作系统 | Windows 10/11 x64（需真实桌面；GUI 使用 Qt Widgets 与 Win32 进程内存访问） |
| 工具链 | Visual Studio 2022（MSVC x64）+ Ninja + CMake ≥ 3.20 |
| 包管理 | vcpkg（默认取 VS2022 自带，可用 `VCPKG_ROOT` 覆盖）——仅用于 gtest |
| Qt | Qt 6.10.2，**独立安装**（不纳入 vcpkg manifest）：Debug 共享版、Release 静态版 |
| 可选 | WSL（`cmd.exe` 作为入口调用 `.bat` 脚本） |

环境变量（脚本内有兜底默认值，跨机使用时按需覆盖）：

```bat
set VCPKG_ROOT=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg
set QT6_DEBUG_DIR=C:/d/ccpplib/qt6.10.2-windows-x86_64-msvc2022-shared_debug
set QT6_RELEASE_DIR=C:/d/ccpplib/Qt_6.10.2-static-Release_msvc2022_64/msvc2022_64
```

## 构建与运行

**编译顺序固定：先静态库，后 GUI**（GUI 用 `find_library` 引用 `psig/out/lib/<配置>/psig.lib`）。

```bash
# 1) 静态库（Release 静态 /MT；Debug 共享 /MDd）
bash psig/script/build_release_wsl.sh      # 或 bash psig/script/build_debug_wsl.sh
#    → psig/out/lib/Release/psig.lib

# 2) GUI（必须与静态库同配置）
bash gui/script/build_release_wsl.sh       # 或 bash gui/script/build_debug_wsl.sh
#    → gui/out/bin/Release/PsigMatcher.exe

# 3) 运行（Release 为静态 Qt，自包含、无控制台窗口）
gui/out/bin/Release/PsigMatcher.exe
```

- Windows 侧等价入口：直接执行 `psig\script\build_release.bat`、`gui\script\build_release.bat`。
- Debug 使用**共享 Qt**：运行时需把 Qt 的 `bin` 加入 `PATH`（`run_*_tests*.bat` 已自动处理）；Debug 保留控制台便于调试，Release 无控制台。
- 预设：`gui/CMakePresets.json` / `psig/CMakePresets.json` 的 `debug`（`x64-windows`，`/MDd`）与 `release`（`x64-windows-static`，`/MT`）。

## 测试

| 层级 | 位置 | 运行方式 |
| --- | --- | --- |
| 静态库单元 + 集成 | `psig/test/unit`、`psig/test/integration` | `cd psig/out/build/release && ctest --output-on-failure` |
| GUI 单元（View / Model / ViewModel） | `gui/test/unit` | `bash gui/script/run_release_tests_wsl.sh`（Debug：`run_debug_tests_wsl.sh`） |
| GUI 集成 + 用户流程 | `gui/test/integration` | 同上（ctest 一并执行） |
| E2E（真实启动 + 注入 + 像素断言） | `gui/test/e2e` | `bash gui/test/e2e/verify_all.sh Release` |

- GUI 测试以 `QT_QPA_PLATFORM=offscreen` 运行，可在无显示器环境执行。
- E2E 顺序固定：清理残留进程 → 启动程序 → 注入真实按键/`WM_CLOSE` → 截客户区 → 像素断言（`analyze.py`）；无需管理员权限。用例清单见 `gui/test/e2e/cases.md`。

## 静态库公共 API（`psig/include/psig.h`）

唯一对外契约；GUI 只能经此调用，静态库不依赖 Qt、**不抛异常**（用返回值/出参表达错误）。

- 进程与扫描：`EnumerateProcesses`、`ProcessIcon`、`DefaultProcessIcon`、`ParsePattern`、`ScanProcesses`（按内存区域并行，可传进度回调/取消标志）、`RequestCancel`
- 特征码表：`BuiltinSignatureColumns`、`ImportSignaturesCsv/Db`、`ExportSignaturesCsv/Db`
- 匹配结果：`ExportResultsCsv/Db/Txt`、`ResultColumnDefs`、`SelectResultKeys`
- 结构体：`ProcessInfo`、`Pattern`、`MatchResult`、`Table`/`TableRow`/`ColumnDef`、`ResultExportOptions`、`ScanRequest`/`ScanProgress`

约定：CSV 读写处理 UTF-8 BOM；`.db` 为 SQLite（后缀 `.db`）；扫描进度回调可能被多个工作线程并发调用，实现须线程安全且不得直接操作界面。

## 设计与流程文档

- `design/v<版本>/gui/`：GUI 侧分析→设计→接口→审查矩阵→E2E 结果（编号 00–35，如 `04_GUI需求分析.md`、`24_ViewModel接口.md`、`31_ViewModel层审查矩阵.md`）。
- `design/v<版本>/gproto/`：静态库侧文档；`design/v<版本>/gui/00_CHANGELOG.md` 记录「本版 = 上一版 + 改动点清单」。
- **已归档版本目录不再原地修改**：每次改动新建 `design/v<版本>/`，版本号与 git tag 一一对应。
- `.flow/` 为流程规范：新建程序用 `带GUI的单体程序的GUI入口系统.md` + `带GUI的单体程序的业务系统静态库.md`；增量修改用 `带GUI的单体程序的添加与修改.md`，需求输入放 `.idea/v<版本>.md`。

## 项目约定

- **分层边界**：View 只画 UI（不 `connect`、不发 signals、不含事件逻辑，可单独编译）；Model 纯净（无业务/格式化）；ViewModel 集中 `connect`、经 View 公开 getter 读控件、经 View 公开方法刷新 UI（不用 `findChild`）；业务能力一律来自静态库公共 API，不在 GUI 内重复实现或编造数据。
- **命名**：生产 exe = `PsigMatcher`（v0.3.2 由 `psig_gui` 改名）；静态库 = `psig`；测试目标 = `psig_test_*` / `psig_target`。
- **脚本管理**：`gui/script/` 按「核心 / 验证 / 排查 / 一次性工具」四类管理，新增脚本必须归入其一并同步 `gui/script/README.md`。
- **提交信息**：`type: content`（chore/docs/feat/fix/test），内容用英文。
- **语言**：界面文案经 `tr()` 并提供中/英翻译（`gui/res/i18n/PsigMatcher_{zh_CN,en_US}.ts`）；中文为源语言。

## 各版本要点

| 版本 | 要点 |
| --- | --- |
| v0.1.0 → v0.2.x | 首个完整版本与后续增量：语言切换、进程图标/列宽、日志 dock、扫描进度与并行、结果列选择等 |
| v0.3.0 | 扫描进度显示真实进度、区域级并行、定时器采样刷新 |
| v0.3.1 | 进程表整行悬停/选中绘制修复、角落全选禁用、特征码 `.db` 携带目标进程名 |
| **v0.3.2（当前）** | 未保存改动**真实检测**（含单元格编辑）+ 持久状态 `*`；导入与保存解耦（仅覆盖确认）；**关闭/退出未保存提示**（菜单「退出」修复为真正生效）；程序改名 **`PsigMatcher.exe`** |

## 常见问题

| 现象 | 处理 |
| --- | --- |
| 配置阶段报「未找到 psig.lib」 | 先编译静态库（`psig/script/build_release_wsl.sh`），并确保 GUI 与静态库配置一致 |
| Debug 版启动报缺少 `Qt6*.dll` | Debug 用共享 Qt，需把 `%QT6_DEBUG_DIR%/bin` 加入 `PATH`（或用 `run_debug_tests*.bat`） |
| Release 版无控制台输出 | Release 为 `WIN32` 子系统的静态 Qt 程序，属预期；排查时可用 Debug 版 |
| 扫描某进程无结果 / 被拒绝 | 目标进程需要足够权限读取其内存；进程表以权限图标提示可访问性 |
| E2E 失败提示缺少编译产物 | E2E 运行 `Release` 产物：先 `bash gui/script/build_release_wsl.sh` |

## 许可证

本项目采用 **MIT** 许可证，详见 [LICENSE](LICENSE)。

```
MIT License

Copyright (c) 2026 Theqiqi
```
