// viewmodel/viewmodel_helpers.h - ViewModel 层内部纯函数（供各 partial 实现文件共享）
#pragma once

#include <string>
#include <utility>
#include <vector>

#include <QDialog>
#include <QDialogButtonBox>
#include <QImage>
#include <QString>

#include "model/types.h"
#include "psig.h"

namespace vmhelp {

// psig 列定义 -> Model 列定义
ColumnDef toModelColumn(const psig::ColumnDef& c);

// psig 匹配结果 -> 结果行
ResultRow toResultRow(const psig::MatchResult& m);

// 结果行 -> psig 匹配结果
psig::MatchResult toPsigResult(const ResultRow& r);

// psig 的 BGRA 位图 -> 独立缓冲的 QImage（ARGB32）
QImage toQImage(const psig::IconBitmap& bmp);

// 结果导出列定义（键, 标题），顺序与 psig 结果列一致
std::vector<std::pair<QString, QString>> resultColumnDefs();

// 结果导出表头：按当前语言（经 tr()）输出；自定义列固定「备注」不随语言（C-05/C-06）
QString resultHeaderForKey(const std::string& key);

// 特征码表内置列标题：按当前语言（经 tr()）；自定义列沿用其标题（不随语言，C-06）
QString signatureHeaderForKey(const QString& key);

// 对话框按钮接线（View 不 connect；统一在 VM 完成）
void wireDialog(QDialog* dlg, QDialogButtonBox* box);

} // namespace vmhelp
