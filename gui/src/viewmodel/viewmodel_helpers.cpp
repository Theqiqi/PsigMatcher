// viewmodel/viewmodel_helpers.cpp - ViewModel 层内部纯函数实现
#include "viewmodel/viewmodel_helpers.h"

#include <QObject>

namespace vmhelp {

ColumnDef toModelColumn(const psig::ColumnDef& c) {
    ColumnDef m;
    m.key = QString::fromStdString(c.key);
    m.title = QString::fromStdString(c.title);
    m.type = QString::fromStdString(c.type);
    m.builtin = c.builtin;
    return m;
}

ResultRow toResultRow(const psig::MatchResult& m) {
    ResultRow r;
    r.patternName = QString::fromStdString(m.patternName);
    r.pid = m.pid;
    r.processName = QString::fromStdWString(m.processName);
    r.address = m.address;
    r.moduleName = QString::fromStdWString(m.moduleName);
    r.moduleBase = m.moduleBase;
    r.matchedBytes = QString::fromStdString(m.matchedBytes);
    r.hitTime = QString::fromStdString(m.hitTime);
    r.custom = QString::fromStdString(m.custom);
    return r;
}

psig::MatchResult toPsigResult(const ResultRow& r) {
    psig::MatchResult m;
    m.patternName = r.patternName.toStdString();
    m.pid = r.pid;
    m.processName = r.processName.toStdWString();
    m.address = r.address;
    m.moduleName = r.moduleName.toStdWString();
    m.moduleBase = r.moduleBase;
    m.matchedBytes = r.matchedBytes.toStdString();
    m.hitTime = r.hitTime.toStdString();
    m.custom = r.custom.toStdString();
    return m;
}

QImage toQImage(const psig::IconBitmap& bmp) {
    if (!bmp.valid()) return {};
    QImage img(reinterpret_cast<const uchar*>(bmp.pixels.data()), bmp.width, bmp.height,
               bmp.width * 4, QImage::Format_ARGB32);
    return img.copy(); // 脱离临时缓冲，持有一份独立数据
}

std::vector<std::pair<QString, QString>> resultColumnDefs() {
    return {{QStringLiteral("pattern_name"), QObject::tr("特征码名")},
            {QStringLiteral("process_name"), QObject::tr("进程")},
            {QStringLiteral("pid"), QObject::tr("PID")},
            {QStringLiteral("address"), QObject::tr("地址")},
            {QStringLiteral("address_dec"), QObject::tr("地址(十进制)")}, // C-01：新增十进制地址列
            {QStringLiteral("module_name"), QObject::tr("模块")},
            {QStringLiteral("module_base"), QObject::tr("模块基址")},
            {QStringLiteral("matched_bytes"), QObject::tr("匹配字节")},
            {QStringLiteral("hit_time"), QObject::tr("命中时间")},
            {QStringLiteral("custom"), QObject::tr("备注")}};             // C-01：自定义 → 备注
}

QString resultHeaderForKey(const std::string& key) {
    if (key == "custom") return QStringLiteral("备注"); // 自定义列不随语言
    for (const auto& p : resultColumnDefs()) {
        if (p.first.toStdString() == key) return p.second;
    }
    return QString();
}

QString signatureHeaderForKey(const QString& key) {
    if (key == QStringLiteral("name")) return QObject::tr("名称");
    if (key == QStringLiteral("pattern")) return QObject::tr("特征码");
    if (key == QStringLiteral("note")) return QObject::tr("备注");
    if (key == QStringLiteral("process")) return QObject::tr("进程");
    return QString();
}

void wireDialog(QDialog* dlg, QDialogButtonBox* box) {
    if (!dlg || !box) return;
    QObject::connect(box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
}

} // namespace vmhelp
