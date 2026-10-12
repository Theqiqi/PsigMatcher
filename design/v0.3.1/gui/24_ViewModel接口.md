# 24 ViewModel 接口（v0.3.1，增量）

> 前置：`22_View层接口.md`、`24`（本文件）、静态库 API 文档（`psig.h`）。
> 说明：ViewModel 持 `MainWindow*`/Model*，集中 `connect`，调静态库 API，经 View 公开方法刷新 UI。

## MainViewModel（增量）

```cpp
class MainViewModel : public QObject {
    Q_OBJECT
public:
    // 沿用（行为增强）
    void importSignaturesFromPath(const QString& path);   // 导入后比对目标进程名，不一致则弹窗（C-05）
    void exportSignaturesToPath(const QString& path);     // 导出时写入 table.targetProcess = m_targetName（C-05）
private:
    // 沿用
    psig::Table tableFromModel() const;                   // 变更：table.targetProcess = m_targetName
    void applyTableToModel(const psig::Table& table);     // 不变
    // 新增（私有）
    bool confirmTargetProcessMismatch(const QString& libProc);  // 弹窗：继续=true / 取消=false
};
```

## 行为变更

### exportSignaturesToPath（C-05）

- 组装 `psig::Table` 后，`table.targetProcess = m_targetName.toStdString()`（无目标进程时为空串，psig 侧不写键）。
- 其余（成功/失败反馈、`m_sigDirty=false`）不变。

### importSignaturesFromPath（C-05）

1. 读表（`psig::ImportSignaturesDb`/`ImportSignaturesCsv`）；失败 → 瞬时红 + 日志红（沿用）。
2. **新增比对**：若 `table.targetProcess` 非空 **且** `m_targetName` 非空 **且** 两者不同 →
   - 弹窗（`confirmTargetProcessMismatch`）：
     - 取消 → 瞬时反馈「已取消导入」+ 日志；**不改当前表**，return；
     - 继续 → 继续第 3 步。
3. `applyTableToModel`、`m_sigDirty=false`、瞬时/日志反馈（沿用）。

## 绑定（install/bind*）

- 无新增 `connect`；`onImportSignatures`/`onExportSignatures` 入口不变。

## 约束满足说明

- 每个 VM 业务方法对应真实静态库 API 调用（见 `25_静态库API调用映射.md`）。
- 后台线程异常经信号回主线程（本版无新增后台任务）；弹窗在主线程执行。
