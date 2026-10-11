# v0.2.3 CHANGELOG

> **v0.2.3 = v0.2.2 + 流程审查机制修正 + psig/GUI 结构重构**
>
> 本版本为「修改/添加」型流程，触发点为对 v0.2.2 完成品的**代码质量/架构评审**。
> 上一版本为 `v0.2.2`（tag: `v0.2.2`，design/v0.2.2/）。已归档版本目录不再原地修改。
> 本次改动全部写入 `design/v0.2.3/`。

## 版本号

- 沿用 v0.2.x 起算约定：修改型流程按次版本 +1（`v0.2.2 → v0.2.3`）。
- 本流程版本号：**v0.2.3**。

## 一、触发背景（评审诊断，含证据）

| # | 问题 | 证据 |
| --- | --- | --- |
| 1 | 流程审查步骤**无量化判定标准**，退化为自我背书 | GUI 入口流程审查段 41 步中 27 步、静态库流程 24 步中 20 步的 `约束` 为空泛模板；步骤 46/47（文件长度/函数体长度）无阈值 |
| 1a | 自证通过的直接实证 | `design/v0.2.2/gproto/20_实现与检查记录.md:31-32` 写「嵌套深度 ✅」「文件长度/函数体长度 ✅」，而 `ScanProcesses` 实测嵌套 6 层 / 函数体 91 行；该文件与 v0.2.1 版逐字节相同（标题仍写 "psig v0.2.1"） |
| 2 | 修改流程审查**塌缩并漏项** | 修改流程步骤 34 检查项不含文件长度/函数体长度 → 历次修改发现不了 `mainviewmodel.cpp` 931 行 |
| 3 | **api 层未按职责拆分** | `design/v0.1.0/gproto/09_模块设计.md:28-32` 明确要求「按职责拆分接口：进程/模式/数据交换模块」；实现却是单文件 `psig/src/api/psig.cpp`（554 行） |

## 二、改动点清单

| 编号 | 改动点 | 类型 | 主要影响 |
| --- | --- | --- | --- |
| R-01 | psig api 层按职责拆分：`api/psig.cpp`(554 行) → `process_scan` / `pattern_api` / `exchange_signatures` / `exchange_results` + 新增跨文件 glue `api/api_internal.h`；`EncodeCells/DecodeCells` 下沉 `data/codec` | 重构 | psig api/data 层、`src/CMakeLists.txt` |
| R-02 | `ScanProcesses` 复杂度达标：抽出 `ScanOneProcess`/`ScanRegion`/`ScanBuffer`/`MakeMatchResult`/`UpdateCarry`，由 91 行/6 层 → 16 行/≤3 层 | 重构 | `psig/src/api/process_scan.cpp` |
| R-03 | GUI `MainViewModel` 按功能域拆分：`mainviewmodel.cpp`(931 行) → 8 个 partial 实现文件（同一声明）+ 新增 `viewmodel_helpers.{h,cpp}` | 重构 | `gui/src/viewmodel/`、`gui/CMakeLists.txt` |
| R-04 | 删除死字段 `std::string m_lastError`（全仓无引用） | 清理 | `gui/src/viewmodel/mainviewmodel.h` |
| R-05 | data 层去 WinAPI：`WideToUtf8`/`Utf8ToWide` 由 `data/codec` 下沉至新增 `atomic/win_encoding`（`data/` 变为纯 std，无 `windows.h`） | 重构（审查发现） | `psig/src/data/codec.*`、`psig/src/atomic/win_encoding.*` |
| R-06 | View `buildMenuAndToolbar`（113 行）拆为 `createActions`/`applyActionIconsAndShortcuts`/`buildMenus`/`buildToolbar` | 重构（审查发现） | `gui/src/view/mainwindow.{h,cpp}` |
| R-07 | algorithm `SplitLine` 控制流 4 层 → 3 层（`continue` 提前返回重构） | 重构（审查发现） | `psig/src/algorithm/csv.cpp` |

> **R-05/R-06/R-07 的由来**：v0.2.3 首次以新的**量化标准**（嵌套 ≤3 / 函数体 ≤80 / 文件 ≤400）审查各层，命中 3 处历史遗留违规。这是新审查机制**实际命中**的实证，非计划内改动，特此单列。
> 全仓扫描（`gui/src` + `psig/src`）确认：修复后无任何函数 >80 行、无任何函数控制流 >3 层、无任何源文件 >400 行。

## 三、流程文档同步修正（前置 chore，先于代码重构）

三份流程文档的审查阶段由「逐项罗列」改为「**按层分步 + 步内矩阵**」，并把公共质量项下沉为各层矩阵的公共行，配**统一量化标准**：

| 指标 | 阈值 | 口径 |
| --- | --- | --- |
| 嵌套深度 | ≤ 3 层 | 控制流块 if/for/while/switch 的嵌套层数，函数体本身不计层；else-if 链不额外计层；lambda 体独立计层 |
| 函数体长度 | ≤ 80 行 | 自起始 `{` 的下一物理行至对应 `}`，含空行与注释 |
| 文件长度 | ≤ 400 行（软目标 200） | 物理行；纯类型/常量/语言表文件可豁免，须在矩阵标注理由 |

矩阵统一 4 列：`检查项 | 判定标准（量化） | 结论 | 证据（文件:行号/计数/grep 命中）`；**结论禁止只写 ✅，必须附证据**。

- GUI 入口流程：审查段 45-85（41 步）→ 45-52（8 步），总步数 123 → 90。
- 静态库流程：审查段 33-56（24 步）→ 33-40（8 步），总步数 78 → 62。
- 修改流程：审查段 31-35 保持 5 步（镜像层结构），补「修改模式：仅审受影响文件」与文件/函数体长度项。

## 四、阈值升级说明（不回改历史）

新阈值（嵌套 ≤3 / 函数体 ≤80 / 文件 ≤400）若套用到历史 design 文档会大面积判 ❌。**本版不回改历史归档文档**，仅在 v0.2.3 起生效；v0.2.3 及以后各层审查矩阵逐行附证据。

## 五、验证结果

见各层审查矩阵（`gui/29-33_*`、`gproto/33-40_*`）与构建/测试记录：

- psig：Debug + Release 构建通过；`psig_unit` + `psig_integration` 全绿。
- GUI：Debug + Release 构建通过；单元/集成/用户流程 5 个测试目标全绿。
