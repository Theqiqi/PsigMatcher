# v0.3.1 CHANGELOG

> **v0.3.1 = v0.3.0 + 进程表悬停/选中绘制修复 + 角落全选禁用 + 特征码库携带目标进程信息**
>
> 本版本为「修改/添加」型流程，输入为 `.idea/v0.3.1.md`。
> 上一版本为 `v0.3.0`（tag: `v0.3.0`，design/v0.3.0/）。已归档版本目录不再原地修改。
> 本次改动全部写入 `design/v0.3.1/`。

## 版本号

- 沿用 v0.2.x 起算约定：修改型流程按次版本 +1（`v0.3.0 → v0.3.1`）。
- 本流程版本号：**v0.3.1**。

## git 现状（流程步骤 1）

- 已初始化；当前分支 `master`；工作区 clean。
- 最近提交：`8087115 docs: finalize v0.3.0 changelog`（tag `v0.3.0`）。
- `.flow/`、`design/` 均已纳入跟踪。无需 `git init`（步骤 2 跳过）、`.gitignore` 已覆盖构建产物（步骤 3 无需增补）。

## 改动点清单（初稿，详见 `gui/01_想法分析.md`）

| 编号 | 改动点（原始想法） | 类型 | 影响层 |
| --- | --- | --- | --- |
| C-01 | 进程表悬停整行高亮委托**绘制越界**（`fillRect` 覆盖整行、超出当前单元格）→ 悬停列变深、覆盖前面列字体 | 修复 | GUI View |
| C-02 | 左键单击后前面的列不随整行选中变色，需移动鼠标才整行变色 | 修复 | GUI View |
| C-03 | 鼠标悬停到某列会覆盖其前面列的字体 | 修复 | GUI View |
| C-04 | 点击表格左上角空白（角落按钮）会全选所有行 → **禁用角落按钮** | 修复 | GUI View |
| C-05 | 特征码导出的 `.db` 未记录目标进程信息，导入时无法发现选错进程 → **导出携带目标进程名（库元数据）；导入时与当前目标不一致则提示，允许继续** | 改进 | psig + GUI |

> C-01/C-02/C-03 为**同一根因**（委托在单元格矩形之外绘制）的三种表现，修复方案统一。

## 范围界定（不做的事）

- 不新建窗口/页面，不改技术栈（Qt6::Widgets / Qt6::Concurrent / MSVC+Ninja / vcpkg）。
- 不改变扫描语义（命中判定、结果字段、结果导出格式均不变）。
- 不改变特征码表的列定义与 CSV 往返语义（C-05 仅新增 `.db` 库级元数据）。
- C-05 采取「**提示但不阻断**」：进程名不一致时弹窗告知，用户可选择继续导入。

## 交付结果（流程完成）

- **代码**：
  - GUI View：新增 `gui/src/view/hoverrowtableview.{h,cpp}`（整行悬停视图，跟踪悬停行并重绘受影响的行）；重写 `hoverrowdelegate.{h,cpp}`（改 `initStyleOption`，仅在自身单元格内设悬停底色，消除越界绘制）；`processselectionpage.cpp` 换用 `HoverRowTableView` 并 `setCornerButtonEnabled(false)`；`signatureeditorpage.cpp`/`resultpage.cpp` 换用 `HoverRowTableView`。
  - GUI ViewModel：`signature.cpp` 导出写 `targetProcess`、导入不一致弹窗（`confirmTargetProcessMismatch`，可继续）；`mainviewmodel.h` 增声明。
  - psig：`psig.h` 的 `Table` 新增 `targetProcess` 字段；`exchange_signatures.cpp` 以 `sig_meta('target_process')` 读写（空值不写、缺键不报错）。
  - 版本号：psig.h 及 GUI 标题/启动日志/关于框更新为 **v0.3.1**。
- **文档**：`design/v0.3.1/gui/` 01–34（含 29–33 层审查矩阵、34 E2E 结果）；`design/v0.3.1/gproto/` 02–40 + 77（静态库侧）。
- **测试**：
  - psig 单元：`test_signatures_exchange.cpp` 新增 4 例（`.db` 往返携带目标进程、空值语义、缺文件报错、CSV 往返不变）→ **psig_unit 通过**。
  - GUI 单元：`View.ProcessPageCornerButtonDisabled`、`View.HoverRowHighlightsWholeRowWithinCells`（像素采样）、`ViewModel.ImportTargetProcessMismatchPrompts` → **psig_test_view / psig_test_viewmodel 通过**。
  - Release 构建：psig **2/2**、GUI **5/5** 全绿。
  - E2E：`verify_all.sh Release` **7/7 通过**；标题 `Process Signature Batch Matcher v0.3.1`；PE 子系统 `WINDOWS`（无控制台）。
- **提交与标签**：每步一次英文提交；最终打 annotated tag `v0.3.1`。

