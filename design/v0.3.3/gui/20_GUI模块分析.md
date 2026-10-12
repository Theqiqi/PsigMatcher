# 20 GUI 模块分析（v0.3.3，增量）

- 版本：v0.3.3
- 模式：修改型。仅列出本版改动涉及的层与文件；未涉及模块引用既有文档（`design/v0.3.2/gui/20_GUI模块分析.md`、`design/v0.3.1/gui/20_GUI模块分析.md`）。

## 一、MVVM 4 层与本版改动归属

```
┌─ gui/ ──────────────────────────────────────────────────────────────┐
│  View       src/view/*            ← ★修改：mainwindow.h/.cpp（标题版本号）│
│  Model      src/model/*           ← 未受影响（无 diff）                  │
│  ViewModel  src/viewmodel/*       ← 未受影响（无 diff）                  │
│  资源/入口   res/*, src/main.cpp  ← ★修改：图标资源、.qrc、main.cpp      │
│  构建        CMakeLists.txt       ← ★修改：启用 RC、生产目标加入 app.rc  │
└───────────────────────────────────────────────────────────────────────┘
┌─ psig/ ─────────────────────────────────────────────────────────────┐
│  静态库     include/psig.h + src/*  ← 未受影响（无源码 diff）            │
└───────────────────────────────────────────────────────────────────────┘
┌─ 仓库根 ─────────────────────────────────────────────────────────────┐
│  README.md（英文默认）/ README.zh-CN.md（新增）  ← ★新增/修改            │
│  gui/script/*（图标生成脚本；验证脚本纳入跟踪）   ← ★新增                │
└───────────────────────────────────────────────────────────────────────┘
```

## 二、改动涉及的文件（精确清单）

| 层 | 文件 | 变更 | 对应改动点 |
| --- | --- | --- | --- |
| 资源 | `gui/res/app.ico` | **新增**（生成物，7 帧） | C-01 |
| 资源 | `gui/res/app.rc` | **新增**（`IDI_APPICON ICON "app.ico"`） | C-01 |
| 资源 | `gui/res/app_<N>.png`（N=16,24,32,48,64,128,256） | **新增**（生成物） | C-01 |
| 资源 | `gui/res/app.png` | **修改**（由生成脚本重新产出） | C-01 |
| 资源 | `gui/res/resources.qrc` | **修改**（收录多尺寸 PNG） | C-01 |
| 入口 | `gui/src/main.cpp` | **修改**（多尺寸 `QIcon` 装配） | C-01 |
| View | `gui/src/view/mainwindow.cpp` | **修改**（标题版本号 → v0.3.3） | C-01/版本 |
| 构建 | `gui/CMakeLists.txt` | **修改**（`enable_language(RC)`；资源清单变量） | C-01 |
| 构建 | `gui/src/CMakeLists.txt` | **修改**（生产目标源加入 `app.rc`） | C-01 |
| 脚本 | `gui/script/gen_app_icon.py` | **新增**（图标生成，可复现） | C-01 |
| 脚本 | `gui/script/verify/check_exe_icon.py` | **新增**（PE 图标资源校验） | C-01 |
| 脚本 | `gui/script/README.md` | **修改**（新脚本归入四类管理） | C-01 |
| 测试 | `gui/test/unit/test_view.cpp` | **修改**（新增多尺寸图标用例） | C-01 |
| 文档 | `README.md` / `README.zh-CN.md` | **修改 / 新增** | C-02/C-03 |
| 静态库 | `psig/include/psig.h` | **修改**（仅版本注释） | 版本 |

## 三、未受影响（跳过）

| 模块 | 结论 |
| --- | --- |
| `gui/src/model/*`（3 个 `QAbstractTableModel`） | **未受影响，跳过** |
| `gui/src/viewmodel/*`（8 个 `.cpp` + `mainviewmodel.h`） | **未受影响，跳过** |
| `gui/src/view/`（除 `mainwindow.cpp` 标题行外的其余页面/委托/对话框） | **未受影响，跳过** |
| `gui/test/{integration,target,e2e}/`（除既有回归被执行外无改动） | **未受影响，跳过** |
| `psig/src/*`、`psig/CMakeLists.txt`、`psig/vcpkg.json` | **未受影响，跳过** |

## 四、约束满足说明

- GUI 分为 View / Model / ViewModel / 静态库 4 层，本文件按该 4 层标注改动归属；View 层仍可单独编译（本版对 View 的改动仅一处字符串）。
- 修改模式：已标注改动涉及的层与文件；未涉及模块引用既有文档。
