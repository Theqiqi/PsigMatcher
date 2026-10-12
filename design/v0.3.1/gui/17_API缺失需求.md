# 17 API 缺失需求（v0.3.1）

> 前置：`16_API充分性审查.md`。逐项列出静态库无法满足的能力。

## R-01 特征码库库级目标进程元数据（C-05）

| 项 | 内容 |
| --- | --- |
| 缺失能力 | 特征码库（`.db`）**库级**记录「目标进程名」，并在导入时读回 |
| 期望公共 API | `psig::Table` 新增字段：`std::string targetProcess;`（库级目标进程名，可空）。既有函数签名不变：<br>`bool ExportSignaturesDb(const std::string& path, const Table& table, std::string& err);`<br>`Table ImportSignaturesDb(const std::string& path, std::string& err);` |
| 输入 | 导出：`table.targetProcess`（非空时写入）；导入：`.db` 路径 |
| 输出 | 导入：`table.targetProcess`（库中记录值；缺省空串） |
| 异常要求 | 不抛异常。文件不可读/非本程序库 → 置 `err` 返回空表（沿用）；`sig_meta` 缺表或缺 `target_process` 键 → `targetProcess` 空串，**不算错误** |
| 验收标准 | ① 往返一致：导出 `targetProcess="A.exe"` 再导入得 `"A.exe"`；② 旧库兼容：无该元数据 → 空串且无 err；③ 空 `targetProcess` 导出时不写该键，导入得空串；④ 既有列/行往返不变；⑤ 仅 `.db` 受影响，CSV 路径不变 |

## 交付形式

- 公共头：`psig/include/psig.h`（`Table` 增字段 + 版本串）。
- 实现：`psig/src/api/exchange_signatures.cpp`（`ExportSignaturesDb` 写 `sig_meta('target_process')`；`ImportSignaturesDb` 读回）。
- 测试：`psig` 单元/集成测试新增往返用例。

## 约束满足说明

- 逐项列出缺失能力、期望公共 API、输入输出、异常要求、验收标准。
