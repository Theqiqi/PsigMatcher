# 10 GUI 功能流程图（v0.3.1，增量）

> 模式：修改型。仅新增/修改功能的流程图；其余沿用 `design/v0.2.4/gui/10_GUI功能流程图.md`。

## 一、整行悬停高亮（C-01~C-03，纯绘制，无信号槽）

```
[鼠标在表格 viewport 移动]
        │  QMouseEvent::MouseMove
        ▼
HoverRowTableView::mouseMoveEvent
        │  row = rowAt(event->pos())
        ▼
row == m_hoverRow ? ── 是 ──► 交基类（不重绘）
        │ 否
        ▼
重绘 旧行 + 新行 的视口矩形；m_hoverRow = row
        │
        ▼
viewport 重绘 → 每格 HoverRowDelegate::paint
        │  若 row==hoveredRow 且非选中 → opt.backgroundBrush = 淡色
        ▼
[整行淡色悬停]

[鼠标移出 viewport] → HoverRowTableView::leaveEvent → 清 m_hoverRow → 重绘旧行 → [恢复]
```

## 二、整行选中着色即时生效（C-02）

```
[左键单击某单元格]
        │  Qt 内部 → selectionModel 更新（SelectRows）
        ▼
viewport 重绘受影响行 → HoverRowDelegate::paint（每格各画自身矩形）
        │
        ▼
[整行深色选中，即时铺满，无需移动鼠标]
```

> 说明：本版**不新增**任何信号/槽/connect；流程图展示的是 Qt 内部事件与绘制链。

## 三、特征码导出携带目标进程名（C-05）

```
[文件菜单「导出特征码」] --triggered--> MainViewModel::onExportSignatures
        │  QFileDialog 选 .db 路径（取消→结束）
        ▼
exportSignaturesToPath(path)
        │  table = tableFromModel(); table.targetProcess = m_targetName;
        ▼
psig::ExportSignaturesDb(path, table, err)
        │  成功：sig_meta('target_process') = 目标进程名（非空时）
        ▼
[瞬时反馈「已导出」 + 日志（绿）]  /  失败 → [瞬时反馈（红）+ 日志（红）]
```

## 四、特征码导入比对目标进程名并提示（C-05）

```
[文件菜单「导入特征码」] --triggered--> MainViewModel::onImportSignatures
        │  当前表非空？ → 询问是否保存（沿用；取消→中止）
        ▼
QFileDialog 选 .db（取消→结束）
        ▼
importSignaturesFromPath(path)
        │  table = psig::ImportSignaturesDb(path, err)  （失败→瞬时反馈红+日志）
        ▼
table.targetProcess 非空 且 m_targetName 非空 且 两者不同 ?
        │ 是
        ▼
[弹窗：该库记录目标进程为“X”，当前为“Y”，是否仍要导入？]
        ├── 取消 → [瞬时反馈「已取消导入」+ 日志] → 结束
        └── 继续导入
        ▼
applyTableToModel(table) → m_sigDirty=false → [瞬时反馈「已导入 N 条」+ 日志（绿）]
```

## 五、不受影响（引用既有文档）

- 进程枚举/筛选/图标、扫描开始/取消/进度、结果导出、列管理等流程图：沿用既有文档。
