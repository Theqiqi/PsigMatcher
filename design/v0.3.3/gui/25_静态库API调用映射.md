# 25 静态库 API 调用映射（v0.3.3，增量）

- 版本：v0.3.3
- 前置：`16_API充分性审查.md`、`19_API能力匹配分析.md`、`24_ViewModel接口.md`
- 契约来源：`psig/include/psig.h`

## 一、本版新增映射：**无**

| VM 业务方法 | 对应静态库 API | 本版变化 |
| --- | --- | --- |
| （本版无新增 VM 方法） | — | — |

- 本版**未新增任何**对静态库公共 API 的调用；既有映射（进程枚举、扫描、特征码导入导出、结果导出等）逐条不变，引用 `design/v0.3.2/gui/25_静态库API调用映射.md`。

## 二、既有映射复核（本版未改动，抽样实测）

| VM 方法（既有） | 静态库公共 API（`psig.h` 逐字符号） | 本版是否改动 |
| --- | --- | --- |
| `onRefreshProcesses` | `EnumerateProcesses`、`ProcessIcon`、`DefaultProcessIcon` | ❌ 未改动 |
| `onStartScan` | `ParsePattern`、`ScanProcesses`、`RequestCancel` | ❌ 未改动 |
| `onImportSignatures` / `onExportSignatures` | `ImportSignaturesCsv/Db`、`ExportSignaturesCsv/Db`、`BuiltinSignatureColumns` | ❌ 未改动 |
| `onExportResults` / 列管理 | `ExportResultsCsv/Db/Txt`、`ResultColumnDefs`、`SelectResultKeys` | ❌ 未改动 |

- 证据：`git diff` 未触及 `gui/src/viewmodel/`（除两处版本字符串）；`psig/include/psig.h` 函数声明与 v0.3.2 逐字一致。

## 三、本版引入的「非静态库」设施（明确不冒充能力提供方）

| 设施 | 用途 | 是否出现在 VM 层 | 是否冒充静态库能力 |
| --- | --- | --- | --- |
| MSVC `rc.exe` / `link.exe` | 把 `app.ico` 编入 exe 的 PE 资源节 | ❌ 构建期 | ❌ 不是「能力提供方」，是构建工具 |
| Qt `rcc` | 把 `.qrc` 中的 PNG 编入二进制 | ❌ 构建期 | ❌ 同上 |
| `QApplication::setWindowIcon`（Qt） | 设置应用窗口图标 | ❌ 入口层 | ❌ 外壳 API，非业务能力（且本版无业务能力需求） |

> 规则复核：「匹配对象只能是静态库公共 API，禁止用 OS API 充当能力提供方」——本版无任何能力需求，故不存在冒充；上表设施均属**外壳/构建**范畴且不进入 ViewModel。

## 四、约束满足说明

- 每个 VM 业务方法对应一个真实静态库 API 调用：本版未新增 VM 业务方法，既有对应关系未变（第二节抽样实测）。
- 无对应 API 的情形：本版无（`17_API缺失需求.md` 为空）。
