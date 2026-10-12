# v0.2.4 CHANGELOG

> **v0.2.4 = v0.2.3 + 进程页修复 / 结果导出修复 / 结果导出格式调整**
>
> 本版本为「修改/添加」型流程，输入为 `.idea/v0.2.4.md`。
> 上一版本为 `v0.2.3`（tag: `v0.2.3`，design/v0.2.3/）。已归档版本目录不再原地修改。
> 本次改动全部写入 `design/v0.2.4/`。

## 版本号

- 沿用 v0.2.x 起算约定：修改型流程按次版本 +1（`v0.2.3 → v0.2.4`）。
- 本流程版本号：**v0.2.4**。

## 改动点清单（初稿，详见 `gui/01_想法分析.md`）

| 编号 | 改动点（原始想法） | 类型 |
| --- | --- | --- |
| C-01 | 进程页出现 "unknown process" → 修复 | 修复 |
| C-02 | 进程表鼠标悬停只高亮单个格子 → 应整行高亮；悬停色比选中色淡但同色系 | 改进 |
| C-03 | 英文界面下结果导出 TXT 仍为中文；结果表「备注」列中文、其余英文 → 修复 | 修复 |
| C-04 | 导出特征码表格会串入「匹配结果最前面的文字」 → 修复 | 修复 |
| C-05 | 导出格式按族收敛（特征码→`.db`，结果→`.csv`/`.txt`），两类后缀不重叠，避免误把结果文件当特征码表导入而覆盖丢数据 | 改进 |

## 交付结果（流程完成）

- **GUI 改动**：`model/processtablemodel.cpp`（C-01）、`view/hoverrowdelegate.{h,cpp}`+三张表安装（C-02）、`viewmodel/viewmodel_helpers.cpp`+`viewmodel/result.cpp`（C-03）、`viewmodel/signature.cpp`（C-04）、`viewmodel/result.cpp`+`viewmodel/bind.cpp`+`view/resultpage.{h,cpp}`+`viewmodel/signature.cpp` 过滤器（C-05）；版本串 v0.2.4；i18n `.ts` 同步。
- **psig 改动**：`EnumerateProcesses` 名称来源（快照 `szExeFile` 优先）、`ResultExportOptions::summary`；`data/types.h` 新增 `ProcessEntry`；`atomic/win_process` 新增 `EnumProcessEntries`。
- **构建/测试**：
  - psig Debug/Release 编译通过；`psig_unit` + `psig_integration` 全绿（各 2/2 目标；单元 42 例、集成 18 例）。
  - GUI Debug/Release 编译通过；`psig_test_{view,model,viewmodel,mvvm,userflow}` 全绿（各 5/5）。
  - E2E：`verify_all.sh Release` 7/7 通过（截图证实 C-01 与版本号）。
- **审查**：View/Model/ViewModel/静态库调用/横切 5 张矩阵全部通过（无 ❌）。
- **tag**：`v0.2.4`（annotated，指向最终提交）。
- **不做的事**：不新建窗口/页面、不改技术栈、不引入专有后缀、不删除 psig 公共 API。
