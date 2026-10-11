# 17 原子化 api 分析（psig v0.2.1）

> 前置：`12_接口细化设计.md`、`design/v0.2.0/gproto/17_原子化api分析.md`（基线）

## 分析结论

本次改动**新增原子化 API**（图标提取，平台相关）：

| 原子化能力 | 说明 |
| --- | --- |
| `ExtractProcessIcon(uint32_t pid)` | 取进程可执行文件路径 → `SHGetFileInfoW` 取 HICON → 转 BGRA 位图 |
| `ExtractDefaultIcon()` | `SHGetFileInfoW("a.exe", SHGFI_USEFILEATTRIBUTES)` 取默认 exe 图标 → BGRA 位图 |
| （复用）`ReadTextFile/WriteTextFile` | CSV/TXT 读写（BOM 由接口层拼接/剥离，算法层提供常量） |

## 原子化边界确认

- 原子化层只做「平台调用 + 原始数据转换」，不含「模板」「列选择」等业务语义，也不决定 BOM 写在哪。
- 图标 API 仅负责 Win32 提取与位图格式转换（HICON → BGRA），不理解 GUI 用途。

## 约束满足说明

- 新增平台相关原子化 API（图标）；不引入 Qt；不伪造 API。
