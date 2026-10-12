# 24 ViewModel 接口（v0.2.4，增量）

> 前置：`22_View层接口.md`、`23_Model层接口.md`、`25_静态库API调用映射.md`
> 说明：VM 持 `View*`，集中 connect；经 View 公开 getter 读控件、公开方法更新 UI；不用 findChild。

## 接口（增量）

```cpp
// viewmodel/mainviewmodel.h（节选，仅列本版相关）
class MainViewModel : public QObject {
    Q_OBJECT
public:
    void install(MainWindow* view);
    void importSignaturesFromPath(const QString& path);
    void exportSignaturesToPath(const QString& path);
    void exportResultsToPath(const QString& path);     // 变更：写 summary；不再支持 .db 入口
    void setTemplateText(const QString& text);         // 结果页模板（不参与特征码导出）
    void setExportColumns(const QStringList& keys);
    void applyLanguage(const QString& code, bool feedback = true);

private slots:
    void onExportResultsCsv();                          // 保留
    void onExportResultsTxt();                          // 保留
    // void onExportResultsDb();                        // 移除：结果不再提供 .db 入口（C-05）
    // 其余沿用

private:
    psig::Table tableFromModel() const;                 // 变更：不再写 templateText（C-04）
    void applyTableToModel(const psig::Table& table);   // 变更：不再回填模板（C-04）
    psig::ResultExportOptions exportOptions() const;    // 变更：构造 summary（C-03）
    QString m_templateText;                             // 仅属结果页
};
```

```cpp
// viewmodel/viewmodel_helpers.h（节选）
namespace vmhelp {
QString resultHeaderForKey(const std::string& key);   // 变更：custom 走 tr()（C-03）
}
```

## 关键行为

- `tableFromModel()`（C-04）：构造特征码 `psig::Table` 时**不再**设置 `t.templateText`（保持空）；导出文件不再携带结果页模板文字。
- `applyTableToModel()`（C-04）：导入特征码后**不再**写 `m_templateText`、不再调用 `resultPage()->setTemplateText()`；结果页模板保持原值。
- `exportOptions()`（C-03）：在既有 `columns`/`headers` 基础上新增
  `o.summary = tr("匹配结果 (%1 条)").arg(m_resultModel->rowCount())`（本地化摘要）；`headers` 经 `resultHeaderForKey`（`custom`→`tr("备注")`）随语言。
- `onExportResultsCsv()`/`onExportResultsTxt()`（C-05）：保留；文件对话框过滤器分别为 `*.csv` / `*.txt`。移除 `onExportResultsDb()` 与其菜单动作。
- `importSignaturesFromPath()`/`exportSignaturesToPath()`（C-05）：文件对话框过滤器改为 `*.db`（移除 `*.csv`）。
- `resultHeaderForKey("custom")`（C-03）：由 `QStringLiteral("备注")` 改为 `QObject::tr("备注")`，使英文界面导出列头为 `Note`。

## 约束满足说明

- VM 经 View 公开 getter 读控件、公开方法更新 UI，不用 findChild；每业务方法对应真实 psig API。
- 模板文本仅属结果页；特征码导出/导入与其解耦。
