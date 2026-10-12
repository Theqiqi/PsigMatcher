# 14 API 能力需求（v0.3.1，增量）

> 模式：修改型。聚焦改动点新增的能力需求；C-01~C-04 为纯 GUI 绘制/控件行为，**不涉及静态库能力**。

## 一、C-01~C-04：无静态库能力需求

- 悬停/选中绘制、角落按钮禁用均为 Qt 视图层职责，不经静态库。**不产生 API 需求。**

## 二、C-05：特征码库携带目标进程信息

| 项 | 内容 |
| --- | --- |
| 所需能力 | 特征码库（`.db`）在导出时写入、导入时读回**库级目标进程名**元数据 |
| 期望公共 API | 扩展既有数据交换契约：`psig::Table` 增加库级字段 `std::string targetProcess`；`ExportSignaturesDb` / `ImportSignaturesDb` 负责其持久化（键 `target_process`） |
| 输入 | 导出：`Table{... targetProcess = 目标进程名}`；导入：`.db` 文件路径 |
| 输出 | 导入：`Table.targetProcess` = 库中记录的目标进程名（无记录则空串） |
| 异常要求 | 不抛异常；读取失败/非本程序库 → 置 `err`、返回空表（沿用）；缺 `sig_meta` 或键不存在 → `targetProcess` 为空串（**不视为错误**，兼容旧库） |
| 验收标准 | ① 导出含目标进程名的库，重新导入得到相同 `targetProcess`；② 旧库（无该元数据）导入得到空串且不报错；③ CSV 往返语义不变 |

## 三、能力覆盖判定

- 既有 API：`ExportSignaturesDb` / `ImportSignaturesDb` / `psig::Table` 存在，但 `Table` **无库级进程字段**、DB 读写**不含该元数据**。
- 结论：**部分覆盖**——需扩展 `psig::Table`（公共头）与 `exchange_signatures.cpp`（api 层），属静态库能力补充 → 进入 API 充分性审查与（必要时）静态库流程。

## 四、约束满足说明

- 逐条列出改动点所需能力、期望公共 API、输入输出、异常要求、验收标准。
- 已有 API 可覆盖 → 直接调用；无对应 API → 记入缺失能力，走静态库流程（见 `17_API缺失需求.md`）。
