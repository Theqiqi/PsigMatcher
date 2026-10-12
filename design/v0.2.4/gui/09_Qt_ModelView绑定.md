# 09 Qt Model/View 数据绑定（v0.2.4，增量）

> 模式：修改型。分层绑定沿用：View 持控件、Model 纯数据、ViewModel 经 `View*` 操控并集中 `connect`。仅列出受影响绑定。

## 进程表（`ProcessTableModel`）
- 列（不变）：0=PID、1=名称、2=路径、3=可访问。
- **名称列（C-01）**：`data()` 不再依赖 `tr("未知进程")`；名称由 psig 进程枚举提供（Toolhelp `szExeFile` 优先，映像路径兜底），正常情况非空。空名兜底仅作极端保留。
- 图标（`DecorationRole`，列1）：仅 `r.accessible==true` 时注入（沿用 v0.2.2 C-09）。

## 特征码表（`SignatureTableModel`）
- 列来自 `m_columns`（含 builtin 标记）；表头 builtin 随语言、自定义列不随语言（沿用 v0.2.2 C-06）。
- **导出/导入（C-04）**：`tableFromModel` **不再**写入模板文本（`t.templateText` 不再取自结果页 `m_templateText`）；`applyTableToModel` **不再**回填结果页模板文本。
- **格式入口（C-05）**：导出/导入对话框过滤器仅 `*.db`。

## 结果表（`ResultTableModel`）
- 列（不变）：0..9 特征码名/进程/PID/地址(十进制)/地址/模块/模块基址/匹配字节/命中时间/备注。
- **列9 表头（C-03）**：`resultHeaderForKey("custom")` 走 `tr("备注")`（英文 `Note`），不再硬编码中文。
- 导出列选项（`resultColumnDefs`）：沿用 v0.2.2（含 `address_dec`）。
- **格式入口（C-05）**：结果导出对话框过滤器仅 `*.csv` / `*.txt`，移除 `.db`。

## 表格交互绑定（C-02）
- 三张 `QTableView` 统一：`setMouseTracking(true)` + 自定义 delegate（`paint` 中判断 `State_MouseOver` 绘制整行淡色背景）；不改变 `selectionBehavior=SelectRows` 语义。
- 悬停态不写入 Model、不触发信号。

## 静态库调用映射
- 结果导出：`psig::ExportResultsCsv/Txt(path, results, opt)`，`opt.headers` 传翻译后表头、摘要随语言（C-03）。
- 特征码导出：`psig::ExportSignaturesCsv/Db(path, table)`，`table` **不含模板文本**（C-04）。
- 进程枚举：`psig::EnumerateProcesses()` 名称来源修正（C-01）。
