# 17 原子化 api 分析（psig v0.2.4）

> 前置：`12_接口细化设计.md`

## 原子化 api 细节

### `std::vector<data::ProcessEntry> EnumProcessEntries();`
- 平台：`CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS)` + `Process32FirstW/NextW`。
- 输入：无。
- 输出：`{pid, name=szExeFile}` 列表。
- 异常：快照失败返回空表；不抛异常。
- 说明：由既有 `EnumProcessIds()` 升级而来（携带名称）；`EnumProcessIds` 可移除或保留为薄包装。

### `bool ProcessNamePath(pid, name, path);`
- 沿用：`OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION)` + `QueryFullProcessImageNameW`。
- 本次角色：**名称兜底** + 路径来源（受保护进程可能失败 → 名/路径为空，由快照名兜底）。

## 原子化 api 列表（受影响）
| 原子 api | 变更 |
| --- | --- |
| `EnumProcessEntries` | 新增/升级 |
| `ProcessNamePath` | 沿用（角色调整） |
