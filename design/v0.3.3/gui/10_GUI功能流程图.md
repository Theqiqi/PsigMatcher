# 10 GUI 功能流程图（v0.3.3，增量）

- 版本：v0.3.3
- 模式：修改型。仅新增本版改动点对应的流程图（Qt 信号/槽触发路径）；其余沿用 `design/v0.3.2/gui/10_GUI功能流程图.md`。

## 一、F-ICON-01 exe 图标资源（构建期，无运行期信号）

```
[开发者] bash gui/script/gen_app_icon.py
        │
        ├─ 生成 gui/res/app.ico         （7 帧：16/24/32/48/64/128/256）
        └─ 生成 gui/res/app_<N>.png     （7 个文件，供 Qt 使用）
        │
        ▼
[CMake 配置] project(...) + enable_language(RC)   → 启用 MSVC 资源编译器 rc.exe
        │
        ▼
[构建] PsigMatcher 目标源含 res/app.rc
        │
        ├─ rc.exe 编译 app.rc → app.res（含 RT_GROUP_ICON / RT_ICON）
        └─ link.exe 把 .res 链接进 PsigMatcher.exe
        │
        ▼
[产物] gui/out/bin/<配置>/PsigMatcher.exe   ← PE 资源节内含 7 尺寸图标
        │
        ▼
[Windows 外壳] 读取 RT_GROUP_ICON → 资源管理器/任务栏固定项显示程序图标
```

- 触发者：**开发者/构建系统**（非终端用户），故无 Qt 信号；错误以构建失败或脚本非零退出码终止。

## 二、F-ICON-02 多尺寸窗口图标（启动期）

```
QApplication app(argc, argv);
        │
        ├─► app.setStyleSheet(...)                       [既有]
        │
        └─► ★ app.setWindowIcon(icon)                     [本版修改]
                 │
                 │  QIcon icon;                         （View 层之外的入口装配，见 21/24）
                 │  for (n in {16,24,32,48,64,128,256})
                 │      icon.addFile(":/app_<n>.png", QSize(n,n));
                 │
                 ▼
            QIcon 持有 7 个 pixmap
                 │
                 ▼
            MainWindow view;  view.show();
                 │
                 └─► Qt 按目标 DPI/场景选帧 → setWindowIcon → WM_SETICON
                          ├─ 标题栏 (16)      ✔ 原生帧
                          ├─ 任务栏 (32/48)   ✔ 原生帧
                          └─ Alt-Tab (48/64)  ✔ 原生帧
```

- 信号/槽：无（`QApplication` 级一次性装配）。
- 数据来源：Qt 资源系统 `:/app_<n>.png`（编译期由 `rcc` 从 `resources.qrc` 生成）。

## 三、F-ICON-03 图标生成与校验（开发期命令行流程）

```
python3 gui/script/gen_app_icon.py
        │
        ├─ 渲染几何规则：|u − v| ≤ 5/32（深蓝底 #2A4A7A / 亮蓝带 #6FA8E0）
        │
        └─ 写出：res/app.ico（多尺寸 ICO） + res/app_<N>.png（16..256）
                │
                ▼
python3 gui/script/verify/check_exe_icon.py <exe>  ──► 解析 PE 资源节
        │
        ├─ RT_GROUP_ICON 条目数 == 7 ？尺寸集合 == {16,24,32,48,64,128,256} ？
        └─ RT_ICON 载荷与 res/app.ico 对应帧字节一致 ？
                │
                ├─ 全部满足 → 打印 PASS，退出码 0
                └─ 任一不满足 → 打印 FAIL + 实测值，退出码非 0
```

## 四、F-DOC-01/02 双语 README（仓库文档流程）

```
[访问者] 打开 https://github.com/Theqiqi/PsigMatcher
        │
        ▼
GitHub 渲染 README.md（★英文，默认）
        │
        └─ 首行：English | [简体中文](README.zh-CN.md)
                    │
                    └──► 点击 → GitHub 渲染 README.zh-CN.md（简体中文）
                              │
                              └─ 首行：[English](README.md) | 简体中文
                                          │
                                          └──► 返回 README.md（英文）
```

- 无信号/槽；双向链接构成唯一的「导航图」，无死链（`git ls-files` 校验）。

## 五、既有功能流程图

- 进程刷新、目标进程选择、特征码编辑/导入/导出、扫描（含取消）、结果导出、语言切换、未保存提示：**未受影响**，引用 `design/v0.3.2/gui/10_GUI功能流程图.md`。

## 六、约束满足说明

- 已为本版新增/修改功能补出 Qt 信号/槽触发路径（其中启动期/构建期流程明确标注无信号，触发者为 `QApplication` 装配、`rc.exe`/`link.exe` 与脚本）。
- 正常路径见上图；错误路径与取消路径见 `11_GUI执行路径.md`。
