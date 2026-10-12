# 17 API 缺失需求（v0.2.4）

> 前置：`16_API充分性审查.md`
> 目标：逐项列出静态库无法满足的能力、期望公共 API、输入输出、异常要求、验收标准。

## 缺失能力清单

### M-1：进程名来源修正（C-01）

- **期望公共 API**：`std::vector<ProcessInfo> EnumerateProcesses();`（**签名不变**，行为修正）。
- **行为**：`ProcessInfo::name` 一律为真实进程名——优先取 Toolhelp 快照 `PROCESSENTRY32W.szExeFile`（无需打开进程即可得名，与任务管理器一致）；快照名为空时回落 `QueryFullProcessImageNameW`（`ProcessNamePath`）；`path` 仍由 `ProcessNamePath` 提供（可空）。
- **输入**：无。
- **输出**：进程列表，`name` 非空。
- **异常要求**：个别进程取路径/权限失败不影响名称；不抛异常。
- **验收标准**：
  - 任意枚举结果 `ProcessInfo::name` 非空；
  - 受保护进程（如 `Registry`、`MemCompression`、`Secure System`）名与任务管理器一致；
  - `accessible` 语义与既有行为一致（不影响扫描判定）。

### M-2：结果导出摘要随语言（C-03）

- **期望公共 API**：`ResultExportOptions` 新增字段
  ```cpp
  std::string summary;   // 结果摘要文案（含数量，可空）；非空时替换 TXT 默认摘要行
  ```
- **行为**：`ExportResultsTxt` 若 `options.summary` 非空 → 以该串作为摘要行；为空 → 回落现默认中文 `"匹配结果 (N 条)"`（向后兼容）。
- **输入**：调用方本地化的完整摘要串（如 `"Results (3)"`）。
- **输出**：TXT 摘要行为该串；行格式与其余内容不变。
- **异常要求**：空串不报错，走默认；不改变 `templateText`/表头逻辑。
- **验收标准**：
  - 传 `summary="Results (N)"` → TXT 摘要为英文；
  - 不传 → 摘要为中文默认（兼容既有调用）；
  - CSV/DB 导出行为不变（`summary` 仅作用于 TXT）。

## 期望的 API 变更摘要（供静态库流程定稿）

```cpp
namespace psig {
// M-1：进程名来源修正（签名不变，行为变化）
std::vector<ProcessInfo> EnumerateProcesses();   // name 优先 Toolhelp szExeFile

// M-2：结果导出摘要随语言（字段扩展）
struct ResultExportOptions {
    std::string templateText;
    std::vector<std::string> columns;
    std::vector<std::string> headers;
    std::string summary;   // 新增：TXT 摘要文案（可空）
};
bool ExportResultsTxt(path, results, err, options);   // 非空 summary 优先
}
```

## 约束满足说明

- 逐项列出缺失能力、期望公共 API、输入输出、异常要求、验收标准（M-1、M-2）。
- 静态库完成契约返回前，GUI 停止 ViewModel 层相关编码（不得用模拟数据/假接口）。
