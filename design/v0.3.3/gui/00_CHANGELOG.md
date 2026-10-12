# v0.3.3 CHANGELOG

> **v0.3.3 = v0.3.2 + 二进制程序文件图标资源 + README 更新 + 中英双语 README（默认英文）**
>
> 本版本为「修改/添加」型流程，输入为 `.idea/v0.3.3.md`。
> 上一版本为 `v0.3.2`（tag: `v0.3.2`，design/v0.3.2/）。已归档版本目录不再原地修改。
> 本次改动全部写入 `design/v0.3.3/`。

## 版本号

- 沿用既有约定：修改型流程按次版本 +1（`v0.3.2 → v0.3.3`）。
- 本流程版本号：**v0.3.3**。

## git 现状（流程步骤 1–3）

- 已初始化；本流程在**新建分支 `dev`** 上进行（`master` 保持与 `origin/main` 同步）；工作区 clean。
- 最近提交：`e1d5cc4 Merge remote initial commit (LICENSE, stub README); keep project README`（tag `v0.3.2` 指向 v0.3.2 流程的最终提交）。
- 步骤 2 跳过（已初始化）；步骤 3 **修复了既有 `.gitignore` 缺陷**：裸 `verify/` 规则连带忽略了源码目录 `gui/script/verify/`（两个验证脚本此前从未被跟踪，见下「流程外修复」）。

## 改动点清单（初稿，详见 `gui/01_想法分析.md`）

| 编号 | 改动点（原始想法） | 类型 | 影响层 |
| --- | --- | --- | --- |
| C-01 | 「给二进制编译的程序文件加上图标资源」→ exe 内嵌多尺寸图标资源，并让窗口/任务栏图标同步为多尺寸 | 新增 | 资源（res/）+ 入口（main.cpp）+ 构建（CMake RC） |
| C-02 | 「更新 README.md」→ 同步 v0.3.2/v0.3.3 现状与图标、修正过期内容 | 修改 | 仓库根文档 |
| C-03 | 「编写中英两版的 README.md，默认显示英文版，文档前方互相表示和引用 English 与简体中文」 | 新增 | 仓库根文档（双语） |

## 流程外修复（步骤 3 顺带）

| 编号 | 问题 | 处理 |
| --- | --- | --- |
| P-01 | `.gitignore` 中裸 `verify/` 规则连带忽略 `gui/script/verify/`，导致 `dump_dependents.bat`、`dump_directives.bat` 两个验证脚本**从未纳入版本控制** | 删除裸规则（`gui/out/verify/` 已由 `out/` 覆盖），脚本纳入跟踪并单独提交 |

## 范围界定（不做的事）

- 不新建程序、不新增窗口/页面，不改技术栈（Qt6::Widgets / Qt6::Concurrent / MSVC+Ninja / vcpkg）。
- **不改静态库 `psig`**（无新 API、无源码改动；仅文档侧记录「本版不改静态库」）。
- 不改界面文案语义、不改交互、不改业务流程（本版无功能行为变化）。
- 不改特征码/结果的导入导出格式与往返语义。
- 不引入安装包/资源编辑器依赖；图标由**仓库内可复现脚本**生成，不提交不可复现的二进制来源。

## 已澄清的设计决策

| 议题 | 决策 |
| --- | --- |
| 图标资源形式 | **PE 资源**：`app.ico`（多尺寸）+ `app.rc`（`IDI_APPICON ICON ...`），经 `enable_language(RC)` 由 MSVC `rc.exe` 编译进 exe |
| 图标尺寸集合 | `16 / 24 / 32 / 48 / 64 / 128 / 256`（覆盖标题栏、任务栏、Alt-Tab、资源管理器大图标） |
| 图标设计来源 | **沿用既有视觉识别**：`gui/res/app.png` 的深蓝底 + 亮蓝对角带（两色），按同一几何规则在各尺寸程序化重绘（32×32 与既有 `app.png` **逐像素一致**） |
| 图标可复现性 | 新增生成脚本 `gui/script/gen_app_icon.py`（Python + Pillow），`app.ico`/`app.png` 均由脚本生成；脚本入库，二进制可重建 |
| 窗口图标 | 同步升级为**多尺寸 `QIcon`**（`app_16/24/32/48/64/128/256.png` 入 `.qrc`），使高 DPI 任务栏/Alt-Tab 图标清晰 |
| 双语 README | **`README.md` = 英文版（默认显示）**，`README.zh-CN.md` = 简体中文版；两版**开头互相链接**（语言切换行） |
| 版本号 | 界面标题随本版更新为 **v0.3.3**（既有约定：每版标题带版本号） |

## 交付结果（流程完成）

- **代码/资源**：
  - 新增 `gui/script/gen_app_icon.py`（图标生成，可复现）、`gui/res/app.ico`、`gui/res/app.rc`。
  - `gui/res/app.png` 由脚本重新生成；`gui/res/resources.qrc` 收录 `app.png` 与 `app_<N>.png` 多尺寸集合。
  - `gui/src/main.cpp`：窗口图标改为多尺寸 `QIcon`（逐个 `addFile`）。
  - `gui/src/CMakeLists.txt`：生产目标加入 `../res/app.rc`；`gui/CMakeLists.txt` 增加 `enable_language(RC)`。
  - 静态库：**无源码改动**；`psig/include/psig.h` 版本注释 → v0.3.3。
- **文档**：`design/v0.3.3/gui/` 00–35（含 29–33 审查矩阵/结果、34 E2E 结果、35 编译环境与包管理）；`design/v0.3.3/gproto/00_CHANGELOG.md`（本版不改静态库）。
- **仓库文档**：`README.md`（英文，默认）、`README.zh-CN.md`（简体中文），开头语言切换互相链接；同步 v0.3.2/v0.3.3 现状、图标说明与验证命令。
- **测试**：
  - 新增用例：单元 `View.WindowIconHasMultipleSizes`（多尺寸 QIcon）；图标资源脚本校验（PE 图标节 + 尺寸集合 + 32×32 与 `app.png` 逐像素一致）。
  - Debug/Release 编译通过；ctest 全量回归；E2E 全量回归（含标题 v0.3.3 与 exe 图标实测）。
- **提交与标签**：每步一次英文提交；最终打 annotated tag `v0.3.3`。

## 版本目录说明

- 修改型流程：从上一版本目录复制受影响文档，本版仅写「改动点」增量，其余内容引用既有版本目录。
- 本目录文档命名沿用 v0.3.2：`01`–`28` 分析/设计，`29`–`33` 审查矩阵/结果，`34` E2E 结果，`35` 编译环境与包管理。
