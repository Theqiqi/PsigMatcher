# 18 原子化 api 设计（psig v0.2.4）

> 前置：`17_原子化api分析.md`

## 原子化 api 流程图（文本）

`EnumProcessEntries()`：
```
CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS)
  └─ INVALID → 返回空表
  └─ ok → Process32FirstW
        └─ 循环：push {th32ProcessID, szExeFile}
              Process32NextW
        └─ CloseHandle
  → 返回 entries
```

`ProcessNamePath(pid)`（沿用）：
```
OpenProcess(QUERY_LIMITED_INFORMATION, pid)
  └─ 失败 → return false（name/path 保持空）
  └─ ok → QueryFullProcessImageNameW → path；name = 末段
        CloseHandle
```

## 数据结构分析
- `data::ProcessEntry{ uint32_t pid; std::wstring name; }`（纯 std）。

## 异常设计
- 全部以返回值表达（空表 / false），不抛异常；句柄成对释放（`CloseHandle`）。
