# 11 API 接口分析文档（psig）

- 版本：v0.1.0
- 前置：`08_功能设计.md`

| 接口 | 前置条件 | 主流程 | 后置条件 | 异常/失败 |
| --- | --- | --- | --- | --- |
| EnumerateProcesses | 无 | 枚举 pid→取名称/路径/可访问→汇总 | 返回进程列表 | 单项失败标记 accessible=false |
| ParsePattern | raw 非空 | 分词→逐 token 生成 byte+mask | 返回 Pattern | 非法→valid=false+error |
| ScanProcesses | pids/patterns 非空 | 逐进程打开→区域→读→匹配→归属 | 返回命中列表 | 单进程/区域失败跳过；取消返回部分 |
| RequestCancel | 无 | 置全局原子取消标志 | 扫描尽快结束 | 无 |
| ImportSignaturesCsv | 文件存在 | 读文本→ParseCsv→校验内置列 | 返回 Table | 失败置 err 返回空表 |
| ImportSignaturesDb | 文件存在 | 打开 SQLite→读 schema+行 | 返回 Table | 非本 schema 置 err |
| ExportSignaturesCsv/Db | Table 有效 | 写列定义+行 | 文件生成 | 失败置 err |
| ExportResultsCsv/Db/Txt | results 可空 | 写结果 | 文件生成 | 失败置 err |
| BuiltinSignatureColumns | 无 | 返回内置列定义 | 列定义 | 无 |

## 关键细节

- 模式串 token：`XX`（精确字节）、`??`（整字节通配）、`?X`/`X?`（半字节通配）。
- 命中地址：进程虚拟地址；模块归属给出 moduleName/moduleBase，未命中模块时为空。
- 结果上限：`ScanRequest.maxResultsPerPattern`（0=不限）。
