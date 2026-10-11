# 04 GUI 需求分析（需求变更文档，v0.2.2）

> 模式：修改型。逐条列出「改动点 → 新增/修改/删除 → 影响窗口/功能」。未提及的功能沿用 v0.2.1。

## C-01 导出结果缺十进制列；「自定义」→「备注」
- **修改**：导出结果 CSV/DB/TXT 的列选择框（ExportColumnsDialog）新增「地址(十进制)」可选项（对应现有结果显示表的「地址(十进制)」列）。
- **修改**：结果表「自定义」列表头改名「备注」；导出列标签、导出文件表头同步改名。
- 影响：ViewModel `resultColumnDefs()`、psig `ResultColumnDefs`（增 `address_dec` 键 + 默认标题）、psig `ResultRowValues`（输出十进制）、`ResultTableModel` 表头。
- 验收：导出列选项中出现「地址(十进制)」；勾选后导出的该列值为十进制地址；结果表「备注」列与导出表头均为「备注」。

## C-02 列管理「键/类型」改为下拉选择
- **修改**：列管理对话框中「键」「类型」两列由自由输入（`QLineEdit`）改为下拉选择（`QComboBox`）。
- 键候选：内置签名列键（`name`/`pattern`/`note`，来自 `psig::BuiltinSignatureColumns()`）；类型候选：`text`/`int`/`enum`（静态库类型词汇）。
- **确认**：新增/编辑列仅修改特征码表列元数据，扫描只读取 `pattern`/`name`，**不影响匹配结果**。
- 影响：View `ColumnManagerDialog`、ViewModel `onManageColumns`。
- 验收：键/类型不可手输，仅可下拉选择；新增列后执行扫描结果不变。

## C-03 进程列表空名称兜底
- **修改**：进程列表对已知系统进程（PID 0=系统空闲进程、PID 4=系统）在 `name` 为空时显示兜底名称（本地化：`tr("系统空闲进程")`/`tr("系统")`）。
- 影响：`ProcessTableModel::data()`（名称列兜底）。
- 验收：PID 0/4 行显示名称而非空白；同时按 C-09 仅当有权限才显示图标。

## C-04 重新选择进程保存提示
- **新增**：触发「重新选择进程」时，弹窗询问是否保存当前特征码表（保存/不保存/取消）。保存走既有导出逻辑（`exportSignaturesToPath`）。取消则中止重新选择。
- 影响：ViewModel `onReselectProcess`。
- 验收：点击重新选择进程 → 弹窗；选「保存」写入文件后进入进程页；选「取消」留在当前页。

## C-05 导出结果表头随语言
- **修改**：导出结果 CSV/DB/TXT 的表头文字跟随当前界面语言（英文 UI → 英文表头）。
- 实现：扩展 `psig::ResultExportOptions` 增加 `headers` 字段；导出函数优先使用调用方提供的表头，缺省回落中文默认。GUI 传 `resultColumnDefs()` 的翻译后标题。
- 影响：psig `ResultExportOptions`/`ExportResults*`、ViewModel `exportResultsToPath`/`exportOptions`。
- 验收：英文 UI 下导出结果文件表头为英文；中文 UI 下为中文。

## C-06 特征码列语言策略
- **修改（显示）**：默认可编辑列（`name`/`pattern`/`note`/`process` 等 builtin）表头随语言变化；用户新增的自定义列（builtin=false）表头**不随语言**，保持用户填写标题。现状已满足，沿用。
- **修改（导出）**：导出特征码 CSV 时，builtin 列表头随界面语言输出（经 key→tr 翻译），自定义列（备注等）标题原样输出不翻译。
- 影响：ViewModel `tableFromModel`/`toPsigColumn`（builtin 经 key 翻译）、psig 导入 `BuiltinKeyFromTitle`（支持英文标题回映射，保证跨语言导入往返）。
- 验收：英文 UI 导出特征码 CSV 表头为英文；中文 UI 为中文；自定义列标题保持不变；跨语言导入仍正确识别 builtin 列。

## C-07 进程列宽
- **修改**：进程表初始列宽——`名称`=基准×2，`PID` 与 `可访问` 同宽（基准），`路径` 取剩余并保留可拉伸/可调。
- 影响：View `ProcessSelectionPage`（构造或 `setModel` 后 `setColumnWidth`）。
- 验收：默认名称列明显更宽；用户仍可拖动调整。

## C-08 日志开关与右键清空
- **修改**：视图菜单新增「历史日志」开关（切换日志停靠窗可见性）；原视图菜单的「清空日志」移除。
- **新增**：日志窗口（`QPlainTextEdit`）右键菜单含「清空日志」项。
- 影响：View `MainWindow`（视图菜单 + 日志 `customContextMenuRequested`）、ViewModel `onClearLog`。
- 验收：视图菜单可开关日志窗；日志窗口右键可清空；视图菜单不再有清空项。

## C-09 权限图标区分
- **修改**：进程表图标仅对**有读写权限**（`accessible==true`）的进程显示；无权限进程不显示图标（`DecorationRole` 返回空）。不阻止选择。
- 影响：ViewModel `startIconLoad`/`applyProcessIcon`（仅 accessible 时注入图标）、`ProcessTableModel::DecorationRole` 现状不变。
- 验收：有权限进程显示图标、无权限进程空白；两类进程均可被选择并进入扫描。

## 优先级与依赖
- 静态库依赖：C-01、C-05、C-06 需先完成 psig 扩展（见 14、gproto/）。
- 纯 GUI：C-02、C-03、C-04、C-07、C-08、C-09 可独立实现。
