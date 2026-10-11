// model/types.h - Model 层数据结构（仅依赖 Qt）
#pragma once

#include <QIcon>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <vector>

// 特征码表列定义（Model 层自有类型，不依赖 psig）
struct ColumnDef {
    QString key;
    QString title;
    QString type; // "text" | "int" | "enum"
    bool builtin = false;
};

// 特征码行（cells 与 columns 一一对应）
struct SignatureRow {
    QStringList cells;
};

// 进程行
struct ProcessRow {
    quint32 pid = 0;
    QString name;
    QString path;
    bool accessible = false;
    QIcon icon;             // 显示属性（由 ViewModel 注入，不参与导入导出）
};

// 匹配结果行
struct ResultRow {
    QString patternName;
    quint32 pid = 0;
    QString processName;
    quint64 address = 0;
    QString moduleName;
    quint64 moduleBase = 0;
    QString matchedBytes;
    QString hitTime;
    QString custom;      // 自定义列（可编辑，随导出携带）
};
