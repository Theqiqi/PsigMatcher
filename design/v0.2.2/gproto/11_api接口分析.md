# 11 api 接口分析（psig v0.2.1）

> 前置：`08_功能设计.md`

## 接口细节

### 变更/新增结构

```cpp
struct IconBitmap {              // 新增
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels; // BGRA（小端 ARGB32），长度 = width*height*4
    bool valid() const;          // width>0 && height>0 && pixels.size()==width*height*4
};
```

### 变更函数

| 函数 | 变更后签名 | 说明 |
| --- | --- | --- |
| ImportSignaturesCsv | `Table ImportSignaturesCsv(const std::string& path, std::string& err)` | 读入后**剥离 BOM** |
| ExportSignaturesCsv | `bool ExportSignaturesCsv(const std::string& path, const Table& t, std::string& err)` | 写入**以 BOM 开头** |
| ExportResultsCsv | `bool ExportResultsCsv(const std::string& path, const std::vector<MatchResult>& r, std::string& err, const ResultExportOptions& opt = {})` | 写入**以 BOM 开头** |

### 新增函数

| 函数 | 签名 | 说明 |
| --- | --- | --- |
| ProcessIcon | `IconBitmap ProcessIcon(uint32_t pid)` | 取指定进程图标（失败 invalid） |
| DefaultProcessIcon | `IconBitmap DefaultProcessIcon()` | 系统默认进程图标（始终有效） |

> 兼容性：CSV BOM 为**行为变更**（无签名变化）；导入端剥离 BOM 保证往返一致与旧文件兼容。`IconBitmap` 为纯 std 数据。

## 约束满足说明

- 接口列表与细节已给出；无伪造 API。
