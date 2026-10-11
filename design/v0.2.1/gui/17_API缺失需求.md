# 17 API 缺失需求（v0.2.1）

> 前置：`16_API充分性审查.md`
> 目标：逐项列出静态库无法满足的能力、期望公共 API、输入输出、异常要求、验收标准。

## 缺失能力清单

### M-1：CSV 导出写 UTF-8 BOM + 导入兼容 BOM

- **期望公共 API**：`ExportSignaturesCsv` / `ExportResultsCsv` 写文件时在**最前**写入 UTF-8 BOM（`EF BB BF`）；`ImportSignaturesCsv` 读入时**剥离**文件头 BOM（若有）。
- **输入**：文件路径 + 数据（不变）。
- **输出**：CSV 文件首 3 字节为 `EF BB BF`；导入时 BOM 不进入首个表头/模板记录。
- **异常要求**：无 BOM 的旧 CSV 仍可正常导入；不可读写置 `err`。
- **验收标准**：
  - 导出结果/特征码 CSV → 首 3 字节 = `EF BB BF`；
  - 含中文表头的 CSV 在 Excel 中打开不乱码；
  - 导出后重新导入 → 表头/模板/行数据逐字一致（BOM 不污染首列）。
  - 带模板记录（`#psig-template,…`）时，BOM 在模板记录**之前**，导入时先剥离 BOM 再解析模板记录。

### M-2：进程图标提取

- **期望公共 API**：
  ```cpp
  struct IconBitmap {
      int width = 0;
      int height = 0;
      std::vector<uint8_t> pixels; // BGRA（小端 ARGB32），长度 = width*height*4
      bool valid() const;
  };
  IconBitmap ProcessIcon(uint32_t pid);      // 指定进程图标
  IconBitmap DefaultProcessIcon();           // 系统默认进程图标
  ```
- **输入**：`pid`（`ProcessIcon`）；无（`DefaultProcessIcon`）。
- **输出**：ARGB/BGRA 位图（尺寸 + 像素字节）；取不到返回 `valid()==false`。
- **异常要求**：进程不存在/无图标/权限不足/提取失败 → 返回 invalid，**不抛异常**；`DefaultProcessIcon` 始终应可返回（内置或系统默认）。
- **验收标准**：
  - 对常见系统进程（如 explorer.exe）返回 `valid()==true` 且 `width>0 && height>0`；
  - 对不存在 pid 返回 invalid；
  - `DefaultProcessIcon()` 返回 `valid()==true`；
  - 位图可直接构造 Qt `QImage::Format_ARGB32`（BGRA 小端）且显示正常。

## 期望的 API 变更摘要（供静态库流程定稿）

```cpp
namespace psig {
// M-1：CSV BOM（无签名变化，行为变化；可另加开关但默认写 BOM）
bool ExportSignaturesCsv(path, table, err);              // 写 UTF-8 BOM
Table ImportSignaturesCsv(path, err);                    // 剥离 BOM
bool ExportResultsCsv(path, results, err, options);      // 写 UTF-8 BOM

// M-2：进程图标
struct IconBitmap { int width; int height; std::vector<uint8_t> pixels; bool valid() const; };
IconBitmap ProcessIcon(uint32_t pid);
IconBitmap DefaultProcessIcon();
}
```

## 约束满足说明

- 逐项列出缺失能力、期望公共 API、输入输出、异常要求、验收标准（M-1、M-2）。
- 静态库完成契约返回前，GUI 停止 ViewModel 层相关编码（不得用模拟数据/假接口）。
