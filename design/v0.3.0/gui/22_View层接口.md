# 22 View 层接口（v0.3.0，增量）

> 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`
> 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect**；暴露控件 getter、公开 UI 方法。

## MainWindow（增量）

```cpp
// view/mainwindow.h（节选，仅列本版相关）
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // 沿用
    void setProgress(int value);                 // <0 隐藏；否则显示并设值
    void setTransientMessage(const QString& text, bool error = false); // 变更：错误态未变则不重设样式表
    QProgressBar* progressBar() const;
    QLabel* transientLabel() const;
    // ...
private:
    bool m_transientError = false;               // C-04：瞬时标签当前错误态
};
```

- `setProgress(int)`：沿用（`value<0` 隐藏进度条；否则显示并 `setValue`）。本版由 VM 定时采样驱动。
- `setTransientMessage(text, error)`（C-04）：**仅当 `error` 与 `m_transientError` 不同**时才 `setStyleSheet`，随后 `setText`；避免连续同态刷新重复重设样式表。
- 不新增控件；状态栏结构（左持久 / 中进度条 / 右瞬时）不变。

## 其余页面

- `ProcessSelectionPage` / `SignatureEditorPage` / `ResultPage`：本版**无改动**（沿用 v0.2.4）。

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层；无 signals/slots/connect；控件经 getter 暴露。
- `setTransientMessage` 的样式表优化属纯 View 绘制职责，不引入业务/静态库依赖。
