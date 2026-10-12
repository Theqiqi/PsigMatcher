# 24 ViewModel 接口（v0.3.3，增量）

- 版本：v0.3.3
- 前置：`04_GUI需求分析.md`、`11_GUI执行路径.md`、`22_View层接口.md`、`21_职责和边界.md`

## 一、本版 ViewModel 接口变化：**无新增/删除/改签名**

| ViewModel 公开面 | 本版变化 |
| --- | --- |
| `MainViewModel::install(MainWindow* view)` | **不变**（`main.cpp` 的五步流程不变） |
| `MainViewModel::eventFilter(QObject*, QEvent*)`（v0.3.2 关闭裁决） | **不变** |
| 全部槽函数（`onRefreshProcesses`/`onStartScan`/`onCancelScan`/`onImportSignatures`/`onExportSignatures`/`onManageColumns`/`onAddSignatureRow`/`onConfirmProcess`/`onReselectProcess`/`onClearResults`/`onShow*`/`onLanguage*`/`onAbout` …） | **不变** |
| `isSignatureDirty()`、持久状态 `*` 标记 | **不变** |
| 静态库 API 调用点 | **不变**（本版无新增调用） |

- 头文件 `gui/src/viewmodel/mainviewmodel.h`：**不变**。
- 唯一改动：两处**文案字符串中的版本号**
  - `gui/src/viewmodel/mainviewmodel.cpp:74`：`tr("进程特征码批量匹配器 v0.3.2 已就绪")` → `v0.3.3`
  - `gui/src/viewmodel/result.cpp:138`：关于框文本 `tr("进程特征码批量匹配器 v0.3.2\n" …)` → `v0.3.3`

## 二、为什么图标装配**不**在 ViewModel（边界论证）

| 判据 | 结论 |
| --- | --- |
| 图标是否由业务数据驱动 | ❌ 否（静态资源，与扫描/特征码无关） |
| 是否需要运行期变更 | ❌ 否（仅启动期一次） |
| 是否属于「VM 槽函数 = 校验→调静态库 API→更新 Model→View 刷新」模式 | ❌ 否（无静态库调用、无 Model 更新） |
| 是否可由 View 公开 getter/方法完成 | ❌ 否（应用级属性，View 内没有对应公开方法，且不应为此新增） |
| 结论 | **在入口（`main.cpp`）装配**，ViewModel 不参与（与既有 `setApplicationName`/`setStyleSheet` 同层） |

## 三、`main.cpp` 装配流程（本版，唯一入口改动）

```cpp
// 1) 应用配置（既有 + 本版）
QApplication app(argc, argv);
QApplication::setApplicationName(QStringLiteral("PsigMatcher"));
QApplication::setOrganizationName(QStringLiteral("psig"));

QFile qss(QStringLiteral(":/style.qss"));
if (qss.open(QIODevice::ReadOnly)) app.setStyleSheet(QString::fromUtf8(qss.readAll()));

// ★ 本版：多尺寸窗口图标（逐个加入，交由 Qt 按 DPI/场景选帧）
QIcon icon;
for (int n : {16, 24, 32, 48, 64, 128, 256})
    icon.addFile(QStringLiteral(":/app_%1.png").arg(n), QSize(n, n));
app.setWindowIcon(icon);

// 2) 固定五步流程（不变）
MainWindow view;
MainViewModel viewModel;
viewModel.install(&view);
view.show();
return app.exec();
```

- 约束复核：`main()` 仍只做「创建 View → 创建 ViewModel → `install` → `show` → `exec`」+ 应用级配置；**未**在入口加入业务逻辑。
- VM 持 `View*` 操控 View、`connect` 集中在 `install()`：本版未改变。

## 四、异常策略（本版）

| 场景 | 策略 |
| --- | --- |
| 某尺寸 PNG 缺失（`.qrc` 漏项/路径错） | `QIcon::addFile` 不产生该帧 → 由单元测试 `View.WindowIconHasMultipleSizes` 断言 `availableSizes()` 捕获；不在运行期弹窗（属构建期问题） |
| PE 图标缺失 | 不产生运行期异常（Windows 显示默认图标）→ 由 `gui/script/verify/check_exe_icon.py` 在验证阶段以非零退出码报告 |
| 跨线程异常 | 本版无新增线程/回调，不适用（既有后台异常经信号回主线程的机制未变） |

## 五、约束满足说明

- 「每个 VM 业务方法对应一个真实静态库 API 调用」：本版 VM 未新增方法，既有映射未变（见 `25_静态库API调用映射.md`）。
- 「后台线程异常经信号/回调传递到主线程，不得跨线程抛异常」：本版不涉及。
- `main.cpp` 流程固定为 View → ViewModel → `install` → `show` → `exec` ✅（见第三节）。
