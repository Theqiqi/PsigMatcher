# 25 静态库 API 调用映射（v0.2.1，增量）

> 前置：`24_ViewModel接口.md`、`design/v0.2.1/gproto/77_静态库完成契约.md`

## 映射表

| VM 行为 | psig 公共 API | 说明 |
| --- | --- | --- |
| 刷新进程 | `EnumerateProcesses()` | 不变 |
| 进程图标 | `ProcessIcon(pid)` | **新增**；失败回退 `DefaultProcessIcon()` |
| 默认图标 | `DefaultProcessIcon()` | **新增**；无图标进程 |
| 开始扫描 | `ParsePattern` + `ScanProcesses` | 不变 |
| 取消扫描 | `RequestCancel` / cancel 标志 | 不变 |
| 初始列 | `BuiltinSignatureColumns()` | 不变 |
| 导入特征码 | `ImportSignaturesCsv/Db(path, err) -> Table` | CSV 自动剥 BOM；读 `templateText` |
| 导出特征码 | `ExportSignaturesCsv/Db(path, table, err)` | CSV 自动写 BOM；`templateText` |
| 导出结果 CSV | `ExportResultsCsv(path, results, err, opt)` | CSV 自动写 BOM；`opt = {templateText, columns}` |
| 导出结果 DB | `ExportResultsDb(path, results, err, opt)` | 同上 |
| 导出结果 TXT | `ExportResultsTxt(path, results, err, opt)` | 同上 |
| 结果自定义列 | `MatchResult.custom` | `ResultRow.custom` ↔ `MatchResult.custom` |
| 结果删行 | —（Model 层 `removeRows`） | 不涉及 psig |
| 列排序 / 筛选 / 显隐 / 语言 / 对话框 | —（View/Model/VM） | 不涉及 psig |

## 转换（VM 私有）

- `IconBitmap → QIcon`（**新增**）：
  ```
  if (!bmp.valid()) return QIcon();           // 由 VM 回退 DefaultProcessIcon()
  QImage img(bmp.pixels.data(), bmp.width, bmp.height, bmp.width * 4,
             QImage::Format_ARGB32);
  return QIcon(QPixmap::fromImage(img.copy())); // copy() 持有独立缓冲
  ```
- `ResultRow → MatchResult` / `MatchResult → ResultRow`：含 `custom`（既有）。
- `Table.templateText` ↔ `m_templateText`（UTF-8）。

## 约束满足说明

- VM 每个业务方法对应真实 psig 公共 API；**图标必须来自 psig**（`ProcessIcon`/`DefaultProcessIcon`），GUI 不调用 Win32。
- 匹配对象仅为 psig 公共 API；未使用 OS API 充当能力提供方；未自行实现缺失能力。
