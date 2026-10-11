# v0.2.1 CHANGELOG

> **v0.2.1 = v0.2.0 + 改动点清单**
>
> 本版本为「修改/添加功能」型流程，输入想法来自 `.idea/v0.2.1.md`。
> 上一版本为 `v0.2.0`（tag: `v0.2.0`，design/v0.2.0/）。已归档版本目录 `design/v0.2.0/` 不再原地修改。
> 本次改动全部写入 `design/v0.2.1/`。

## 版本号

- 沿用 v0.2.0 起算的约定：修改型流程按次版本 +1（`v0.2.0 → v0.2.1`）。
- 本流程版本号：**v0.2.1**（与输入文件名一致）。

## 改动点清单（来自 .idea/v0.2.1.md）

| 编号 | 改动点 | 类型 | 主要影响 |
| --- | --- | --- | --- |
| C-01 | 「选择导出列」对话框 OK/Cancel 点击无效；统一按钮语言 | 修复 | View（ExportColumnsDialog）+ ViewModel 接线 |
| C-02 | 新增中/英双语界面；默认随系统语言，菜单栏可手动切换 | 新增 | View + ViewModel + 资源（.ts/.qm）+ main |
| C-03 | 导出 CSV 中文表头乱码，需修复 | 修复 | 静态库（CSV 写 UTF-8 BOM；导入兼容 BOM） |
| C-04 | 特征码页「列管理」对话框所有按钮（添加/删除/上移/下移/OK/Cancel）无效 | 修复 | View（ColumnManagerDialog）+ ViewModel 接线 |
| C-05 | 「关于」弹窗 OK 按钮无效 | 修复 | View（AboutDialog）+ ViewModel 接线 |
| C-06 | 特征码页「新增行」扩展为「新增一行」与「新增多行」 | 新增 | View + ViewModel + Model |
| C-07 | Release GUI 取消黑色控制台窗口（Debug 保留） | 修改 | 构建配置（WIN32 子系统） |
| C-08 | 特征码页新增行时自动填充目标进程名到「进程」列 | 新增 | ViewModel + Model |
| C-09 | 进程列表启动自动刷新一次；筛选框由顶部移到底部 | 修改 | View + ViewModel |
| C-10 | 筛选框有值时刷新进程会混入不相干进程 | 修复 | ViewModel（刷新后重新应用筛选） |
| C-11 | 点击进程表 PID/名称等列头时按对应列排序 | 新增 | Model（排序）+ View（开启排序） |
| C-12 | 进程选择页显示进程图标（无图标用系统默认图标） | 新增 | 静态库（提供图标）+ Model/View + ViewModel |
| C-13 | 统一 UI 配色为白色（结果页模板输入框/日志框当前为黑色） | 修改 | 资源（style.qss） |
| C-14 | 日志等组件用颜色点缀，统一整体风格 | 修改 | 资源（style.qss）+ View |

## 待确认/开放问题

- C-03（CSV BOM）与 C-12（进程图标）需扩展静态库公共 API，走静态库流程（步骤 16-21 判定与交接）。
- C-02 采用 Qt 标准翻译体系（.ts/.qm + QTranslator），lupdate/lrelease 由 Qt 独立安装自带（步骤 37 记录）。
- C-07 仅 Release 取消控制台，Debug 保留控制台便于调试（经确认）。
