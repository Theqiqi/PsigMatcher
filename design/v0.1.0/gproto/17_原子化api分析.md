# 17 原子化 API 分析文档（psig）

- 版本：v0.1.0
- 前置：`12_接口细化设计.md`

| 原子 API | 底层调用 | 输入 | 输出 | 失败 |
| --- | --- | --- | --- | --- |
| EnumProcessIds | CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS) / EnumProcesses | — | pid[] | 空 |
| OpenForRead | OpenProcess(PROCESS_VM_READ\|PROCESS_QUERY_INFORMATION) | pid | HANDLE | 空句柄 |
| CloseHandle | CloseHandle | HANDLE | — | — |
| ProcessNamePath | QueryFullProcessImageNameW | pid/handle | name,path | 空串 |
| EnumRegions | VirtualQueryEx 循环 | HANDLE | Region[] | 到末端/失败停止 |
| ReadMemory | ReadProcessMemory | HANDLE,base,size | bytes | 空 |
| EnumModules | CreateToolhelp32Snapshot(TH32CS_SNAPMODULE) / EnumProcessModulesEx | pid | ModuleInfo[] | 空 |
| ReadTextFile | std::ifstream | path | text | 失败标志 |
| WriteTextFile | std::ofstream | path,text | bool | false |
| SqliteOpen | sqlite3_open | path | db | 失败 |
| SqliteExec | sqlite3_exec/prepare | db,sql | rows | 失败 |
| SqliteClose | sqlite3_close | db | — | — |

## 边界与注意

- 32/64 位：x64 进程读 x64 目标；对 32 位目标（WOW64）仍可读（地址空间不同），如实返回可读区域。
- 权限：读系统进程可能失败→accessible=false / 跳过。
- 区域过滤：仅 `MEM_COMMIT` 且保护位可读（PAGE_READONLY/READWRITE/EXECUTE_READ/EXECUTE_READWRITE/WRITECOPY/EXECUTE_WRITECOPY）。
