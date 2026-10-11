# 24 ViewModel 接口（v0.2.3，增量）

> 前置：`22_View层接口.md`、`23_Model层接口.md`、`25_静态库API调用映射.md`
> 说明：VM 持 `View*`，集中 connect；经 View 公开 getter 读控件、公开方法更新 UI；不用 findChild。
> **本版变更（R-03/R-04）**：`MainViewModel` 由单文件（931 行）拆为多个 partial 实现文件（**同一声明 `mainviewmodel.h`**），公开 API 与测试零改动；纯函数下沉至 `viewmodel_helpers.{h,cpp}`；删除死字段 `m_lastError`。

## 公开接口（不变）

```cpp
class MainViewModel : public QObject {
    Q_OBJECT
public:
    explicit MainViewModel(QObject* parent = nullptr);
    ~MainViewModel() override;
    void install(MainWindow* view);

    // 可测试路径方法
    void importSignaturesFromPath(const QString& path);
    void exportSignaturesToPath(const QString& path);
    void exportResultsToPath(const QString& path);
    void setTemplateText(const QString& text);
    void setExportColumns(const QStringList& keys);
    QStringList exportColumns() const;
    void applyLanguage(const QString& code, bool feedback = true); // "zh_CN"/"en_US"/""
    // private slots / private 方法签名见 mainviewmodel.h（与 v0.2.2 一致）
};
```

> 公开段签名与 v0.2.2 完全一致；测试与调用方无需改动。

## 实现文件划分（partial，同一声明）

| 文件 | 行数 | 职责域 |
| --- | --- | --- |
| `viewmodel_helpers.{h,cpp}` | 42 / 87 | 匿名 namespace 的 8 个纯函数（`toModelColumn`/`toResultRow`/`toPsigResult`/`toQImage`/`resultColumnDefs`/`resultHeaderForKey`/`signatureHeaderForKey`/`wireDialog`），以 `QObject::tr` 保持 `.ts` 上下文 |
| `mainviewmodel.cpp` | 97 | 构造/析构/`install`/`logTagged`/`refreshPersistentStatus`/`markSignatureDirty`/日志右键与开关 |
| `mainviewmodel_bind.cpp` | 130 | `bind*` 装配 + 右键菜单槽 + `applyInitialColumns` |
| `mainviewmodel_process.cpp` | 155 | 枚举/筛选/选中/重选/图标 |
| `mainviewmodel_signature.cpp` | 175 | 增删行/列管理/导入导出/表格转换 |
| `mainviewmodel_scan.cpp` | 90 | 扫描启动/取消/完成/`collectPatterns` |
| `mainviewmodel_result.cpp` | 184 | 结果导出/列选择/填充/清空/关于/`exportOptions` |
| `mainviewmodel_language.cpp` | 50 | `applyLanguage` + 翻译器装载 |

## 关键变更

- **R-03**：原单文件 931 行按功能域拆为 8 个实现文件；`buildMenuAndToolbar` 式的大函数不存在于 VM（VM 最长函数 `onStartScan`=43 行）。`resultColumnDefs()` 由 VM 成员改为 `vmhelp::resultColumnDefs()` 自由函数（调用点同步为 `vmhelp::`）。
- **R-04**：删除死字段 `std::string m_lastError`（全仓无引用）。
- **tr 上下文**：helpers 保留显式 `QObject::tr(...)`，`.ts` 的 `<name>QObject</name>` 上下文（`psig_gui_en_US.ts:733`）条目不失效。

## 约束满足说明

- VM 经 View 公开 getter 读控件、公开方法更新 UI，不用 findChild；每业务方法对应真实 psig API。
- partial 拆分后同域函数分散在多个 `.cpp`，但共享同一声明；`.h` 不改，无重复定义（构建通过验证）。
- 后台异常经 watcher finished 回主线程（不跨线程抛异常）；图标任务持 `m_alive` 存活标志防析构后回填。
