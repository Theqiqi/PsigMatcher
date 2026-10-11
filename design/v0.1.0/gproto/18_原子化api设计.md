# 18 原子化 API 设计文档（psig）

- 版本：v0.1.0
- 前置：`17_原子化api分析.md`
- 产物：原子化 api 流程图、原子化 api 数据结构分析、原子化 api 异常设计

## 1. 原子化 API 流程图（文字）

```
EnumRegions(h):
  addr = 0
  loop:
    VirtualQueryEx(h, addr, &mbi, sizeof(mbi))
    if 0: break
    if mbi.State==MEM_COMMIT and readable(mbi.Protect):
        emit Region{mbi.BaseAddress, mbi.RegionSize, mbi.Protect}
    addr = mbi.BaseAddress + mbi.RegionSize
```

```
ReadMemory(h, base, size):
  buf.resize(size)
  got = 0
  if ReadProcessMemory(h, base, buf, size, &got): return buf[0:got]
  else: return {}   # 部分失败（跨页保护）时整体跳过该区域
```

## 2. 原子化 API 数据结构分析

- `Region{ u64 base; u64 size; u32 protect; }`
- `ModuleInfo{ wstring name; u64 base; u64 size; }`
- SQLite 行：`vector<string>`。

## 3. 原子化 API 异常设计

| API | 异常 | 处理 |
| --- | --- | --- |
| EnumProcessIds | 快照失败 | 返回空 |
| OpenForRead | 权限不足 | 空句柄（调用方跳过） |
| ReadMemory | 读失败/部分读 | 返回已读部分或空 |
| Sqlite* | 打开/语句失败 | 返回错误码 + 消息 |
| Read/WriteTextFile | IO 失败 | 返回失败标志 |

原则：原子层不抛异常，返回可判定的失败值。
