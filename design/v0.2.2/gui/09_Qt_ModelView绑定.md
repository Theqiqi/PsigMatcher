# 09 Qt Model/View 数据绑定（v0.2.2，增量）

- 分层绑定沿用 v0.2.1：View 持控件、Model 纯数据、ViewModel 经 View* 操控并集中 connect。

## 进程表（ProcessTableModel）
- 列：0=PID、1=名称、2=路径、3=可访问。
- 名称列：`data()` 当 `r.name` 为空且 `r.pid`∈{0,4} 时返回兜底名称 `tr("系统空闲进程")/tr("系统")`（C-03）。
- 图标（DecorationRole，列1）：`r.icon`；**仅当 `r.accessible==true` 时 `startIconLoad` 注入图标，否则为空**（C-09）。
- 列宽（View 侧）：名称×2、PID=可访问、路径剩余拉伸（C-07）。

## 特征码表（SignatureTableModel）
- 列来自 `m_columns`（含 builtin 标记）。
- 表头：`headerData()` 对 builtin 键（name/pattern/note/process）返回 `tr(...)`；自定义列返回 `m_columns[i].title`（不随语言，C-06）。
- `retranslate()` 重发 `headerDataChanged` 仅影响 builtin 列。
- 导出：`tableFromModel` 对 builtin 列按 key 经 `tr()` 翻译标题后传给 psig；自定义列标题原样（C-06）。

## 结果表（ResultTableModel）
- 列 0..9：特征码名/进程/PID/地址(十进制)/地址/模块/模块基址/匹配字节/命中时间/备注。
- 列9 表头：`tr("备注")`（C-01 改名，原「自定义」）。
- 导出列选项（`resultColumnDefs`）：新增 `address_dec` → `tr("地址(十进制)")`，与显示列3 对应（C-01）。

## 静态库调用映射
- 结果导出：`psig::ExportResultsCsv(path, results, opt)`，opt.columns 含 `address_dec`；opt.headers 传翻译后表头（C-05）。
- 特征码导出：`psig::ExportSignaturesCsv(path, table)`，table.columns[].title 由 GUI 按语言准备（C-06）。
