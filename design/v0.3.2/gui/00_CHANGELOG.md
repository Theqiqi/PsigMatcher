# v0.3.2 CHANGELOG

> **v0.3.2 = v0.3.1 + 特征码表「未保存」真实检测修复 + 导入与保存解耦 + 关闭程序未保存提示 + 程序正式改名 PsigMatcher**
>
> 本版本为「修改/添加」型流程，输入为 `.idea/v0.3.2.md`。
> 上一版本为 `v0.3.1`（tag: `v0.3.1`，design/v0.3.1/）。已归档版本目录不再原地修改。
> 本次改动全部写入 `design/v0.3.2/`。

## 版本号

- 沿用既有约定：修改型流程按次版本 +1（`v0.3.1 → v0.3.2`）。
- 本流程版本号：**v0.3.2**。

## git 现状（流程步骤 1）

- 已初始化；当前分支 `master`；工作区 clean。
- 最近提交：`189ea72 docs: finalize v0.3.1 changelog`（tag `v0.3.1`）。
- `.flow/`、`design/` 均已纳入跟踪。无需 `git init`（步骤 2 跳过）；
  `.gitignore` 已覆盖 `out/`、`*.exe/*.lib/*.obj/*.log`、`.vs/`、`.idea/`、`vcpkg_installed`、`verify/` 等（步骤 3 无需增补）。

## 改动点清单（初稿，详见 `gui/01_想法分析.md`）

| 编号 | 改动点（原始想法） | 类型 | 影响层 |
| --- | --- | --- | --- |
| C-01 | 「未保存」不是真的检测，而是固定条件（导入前用 `rowCount()>0`；单元格编辑后 `m_sigDirty` 仍为 false） → **真实脏标记** | 修复 | GUI Model 信号 + ViewModel |
| C-02 | 未保存提示里点「保存」→ 弹另存为 → 保存完紧接着又弹导入文件对话框，把「导入」与「保存/导出」两个流程缝在一起 → **导入与保存解耦** | 修改 | GUI ViewModel（`onImportSignatures`） |
| C-03 | 关闭程序时无未保存提示（`m_quitAction` 也未连接，菜单「退出」实为死项） → **新增关闭提示 + 连接退出** | 新增 | GUI ViewModel（事件过滤器）+ View（公开方法） |
| C-04 | 程序名 `psig_gui.exe` 像 demo 名 → **改名 `PsigMatcher.exe`** | 修改 | 构建目标 + 入口 + 资源/脚本/测试 |

## 范围界定（不做的事）

- 不新建程序、不新增窗口/页面，不改技术栈（Qt6::Widgets / Qt6::Concurrent / MSVC+Ninja / vcpkg）。
- **不改静态库 `psig`**（无新 API、无源码改动；仅文档侧记录「本版不改静态库」）。
- 不改变特征码/结果的导入导出文件格式与往返语义（沿用 v0.3.1）。
- 不引入「当前文件路径 / 静默覆盖保存」概念；保存一律走「另存为」文件对话框。
- 「重新选择进程」提示保留既有语义（保存 / 不保存 / 取消）。

## 已澄清的设计决策（用户确认）

| 议题 | 决策 |
| --- | --- |
| C-01 未保存判定 | **真实脏标记**：连接模型 `dataChanged`，单元格编辑计入未保存；提示一律以 `m_sigDirty` 判定（替换导入前的 `rowCount()>0`） |
| C-01 可见性 | 未保存时持久状态区追加 `*` 标记（`就绪 … 特征码 N *`），使真实状态可见 |
| C-02 导入提示 | **仅覆盖确认**：真有未保存改动时提示 [继续导入 / 取消]，**不再提供保存/导出选项**；保存/导出只属于导出流程 |
| C-03 关闭提示 | **新增**：关闭/退出时若有未保存改动，提示 [保存 / 不保存 / 取消]；「取消」阻止关闭 |
| C-04 程序名 | **`PsigMatcher.exe`**；界面标题仍为「进程特征码批量匹配器」（版本号随之更新为 v0.3.2） |

## 交付结果（流程完成）

- **代码**：
  - GUI View：`mainwindow.h` 新增 `quitAction()` getter（C-03）；`mainwindow.cpp` 标题 → v0.3.2。
  - GUI ViewModel：`bind.cpp` 新增 `dataChanged→markSignatureDirty`（C-01）与 `quitAction→close`（C-03）两处 connect；`signature.cpp` 的 `onImportSignatures` 改为**仅覆盖确认**（C-02）、导出/导入成功后刷新持久状态；`process.cpp` 的 `askSaveBeforeReselect` → `askSaveSignatureChanges(bool)`（关闭与重选共用，C-01/C-03）；`mainviewmodel.{h,cpp}` 新增 `eventFilter`（关闭裁决）、`isSignatureDirty()`、未保存标记 `*`。
  - 改名（C-04）：CMake 目标/`project()` → `PsigMatcher`、入口 `setApplicationName("PsigMatcher")`、i18n 资源名 → `PsigMatcher_{zh_CN,en_US}.ts`（装载路径同步）、测试 target → `psig_target`、校验/E2E 脚本与 `style.qss` 注释同步。
  - 静态库：**无源码改动**；`psig/include/psig.h` 版本注释 → v0.3.2。
- **文档**：`design/v0.3.2/gui/` 00–35（含 29–33 审查矩阵/结果、34 E2E 结果、35 编译环境与包管理）；`design/v0.3.2/gproto/00_CHANGELOG.md`（本版不改静态库）。
- **测试**：
  - 新增用例：`View.WindowTitleCarriesVersion`；`ViewModel.SignatureDirtyTracksRealEditsAndExportClearsIt`、`ViewModel.ImportPromptsOverwriteWithoutSaveOptions`、`ViewModel.ClosePromptsOnUnsavedChanges`、`ViewModel.ClosePromptButtonSet`；`UserFlow.UnsavedStateLifecycleWithRealFiles`。
  - Debug：编译通过，ctest **5/5** 通过；Release：编译通过（`out/bin/Release/PsigMatcher.exe`），ctest **5/5** 通过。
  - E2E：`verify_all.sh Release` **8/8 通过**；标题实测 `Process Signature Batch Matcher v0.3.2`；关闭保存提示实测弹出（diff=3.79%）后以 `Ctrl+D` 选「不保存」退出码 `0`。
- **提交与标签**：每步一次英文提交；最终打 annotated tag `v0.3.2`。

## 版本目录说明

- 修改型流程：从上一版本目录复制受影响文档，本版仅写「改动点」增量，其余内容引用既有版本目录。
- 本目录文档命名沿用 v0.3.1：`01`–`28` 分析/设计，`29`–`33` 审查矩阵/结果，`34` E2E 结果。
