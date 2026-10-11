# v0.2.3 gproto CHANGELOG（psig 侧）

> 版本级背景与流程文档修正见 `design/v0.2.3/gui/00_CHANGELOG.md`。本文件只记录 psig 静态库侧改动与审查范围。

## psig 侧改动点

| 编号 | 改动点 | 影响文件 |
| --- | --- | --- |
| R-01 | api 层按职责拆分（单文件 554 行 → 4 个职责文件 + 内部 glue 头）；`EncodeCells/DecodeCells` 下沉 `data/codec` | `api/{process_scan,pattern_api,exchange_signatures,exchange_results}.cpp`、`api/api_internal.h`、`data/codec.{h,cpp}`、`src/CMakeLists.txt` |
| R-02 | `ScanProcesses` 复杂度达标（91 行/6 层 → 16 行/≤3 层） | `api/process_scan.cpp` |
| R-05 | data 层去 WinAPI：`WideToUtf8/Utf8ToWide` 下沉 `atomic/win_encoding` | `data/codec.{h,cpp}`、`atomic/win_encoding.{h,cpp}`、`api/exchange_results.cpp`、`test/unit/{test_codec,test_win_encoding}.cpp`、`src/CMakeLists.txt` |
| R-07 | algorithm `SplitLine` 控制流 4 层 → 3 层（`continue` 提前返回重构） | `algorithm/csv.cpp` |

## 本版审查范围（静态库流程步骤 33-40）

- **33 data 层**：`data/codec` 有改动（新增 `EncodeCells/DecodeCells`、移出宽窄转换）→ 审查。
- **34 algorithm 层**：`algorithm/csv.cpp` 有改动（R-07 拉平 `SplitLine`）→ 审查。
- **35 atomic 层**：新增 `atomic/win_encoding` → 审查。
- **36 api 层**：R-01/R-02 主战场 → 审查。
- **37 层级与模块边界 / 38 规范符合性 / 39 结构与目录规范 / 40 测试非空**：全量核对。

## 编译与测试

- Debug / Release 均构建通过；`psig_unit`（含新增 `test_win_encoding`）+ `psig_integration` 全绿。
