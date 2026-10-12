# 36 api 层审查矩阵（psig v0.3.1）

- 审查对象：`psig/src/api/exchange_signatures.cpp`（本版受影响；其余 api 文件未受影响）
- 量化标准：嵌套 ≤3、函数体 ≤80 行、文件 ≤400 行、单一职责、错误后提前 return、只经公共头调用、不依赖 Qt。

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数） |
| --- | --- | --- | --- |
| 文件长度 | ≤400 行 | ✅ | `exchange_signatures.cpp`=180 行 |
| 函数体长度 | 任一 ≤80 行 | ✅ | `ImportSignaturesDb`（`:72-120`）=49；`ExportSignaturesDb`（`:138-178`）=41；`ExportSignaturesCsv`（`:122-136`）=15 |
| 嵌套深度 | 控制流块 ≤3 | ✅ | `ExportSignaturesDb`：`if(ok)`→`for`→`if(!ok)break` = 3 层（`:169-174`）；`ImportSignaturesDb`：`if(TableExists)`→`if(Query)` = 2 层（`:103-116`） |
| 单一职责 | 一个函数只做一件事 | ✅ | `ExportSignaturesDb` 只负责写库；`ImportSignaturesDb` 只负责读库 |
| 错误后提前 return | 非法状态立即返回 | ✅ | 打开失败 `if(!db) return false;`（`:141`）；缺表 `return table;`（`:80`）；查询失败 `return table;`（`:86,:95`） |
| 头文件越界 | api 只经公共头/下层内部头 | ✅ | `#include "psig.h"` + `algorithm/csv.h`、`api/api_internal.h`、`atomic/file_io.h`、`atomic/sqlite_io.h`、`data/codec.h`（无 Qt） |
| 不依赖 Qt | 无 Qt 头 | ✅ | `grep -n '#include' exchange_signatures.cpp` 无 `<Q...>` |
| 公共 API 真实性 | 新增符号在 `psig.h` 命中 | ✅ | `Table::targetProcess`（`psig.h:80`）；`ExportSignaturesDb`/`ImportSignaturesDb` 签名未变 |
| 无重复实现 | 复用既有 atomic 能力 | ✅ | 未新增 atomic API；调用既有 `SqliteRun`/`SqliteQuery`/`SqliteTableExists` |
| 向后兼容 | 旧库/空值不报错 | ✅ | 空 `targetProcess` 不写键（`:157`）；缺表/缺键 → 空串（`:103-116`）；测试 `DbEmptyTargetProcessYieldsEmpty` 通过 |

## 结论

api 层全部检查项通过（无 ❌），无需回退。
