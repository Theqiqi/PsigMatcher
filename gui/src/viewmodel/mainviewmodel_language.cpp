// viewmodel/mainviewmodel_language.cpp - ViewModel：语言切换（装载翻译器 + 重译界面）
#include "viewmodel/mainviewmodel.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "view/mainwindow.h"

void MainViewModel::applyLanguage(const QString& code, bool feedback) {
    m_language = code;
    // 解析实际生效语言：空 = 跟随系统（中文为源语言，非中文回退英文）
    QString effective = code;
    if (effective.isEmpty()) {
        effective = QLocale().name().startsWith(QStringLiteral("zh"))
                        ? QStringLiteral("zh_CN")
                        : QStringLiteral("en_US");
    }

    if (m_translator) {
        QApplication::removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
    // 源语言为中文：仅非中文目标需要装载翻译
    if (effective != QStringLiteral("zh_CN")) {
        auto* translator = new QTranslator(this);
        if (translator->load(QStringLiteral(":/i18n/psig_gui_") + effective + QStringLiteral(".qm"))) {
            QApplication::installTranslator(translator);
            m_translator = translator;
        } else {
            delete translator;
        }
    }

    if (m_view) {
        m_view->setLanguageChecked(code);
        m_view->retranslateUi();
    }
    if (m_sigModel) m_sigModel->retranslate();
    if (m_procModel) m_procModel->retranslate();
    if (m_resultModel) m_resultModel->retranslate();

    if (feedback && m_view) {
        const QString label = (effective == QStringLiteral("zh_CN")) ? QStringLiteral("中文")
                                                                     : QStringLiteral("English");
        m_view->setTransientMessage(tr("语言已切换为 %1").arg(label));
        logTagged(QStringLiteral("settings"), tr("语言 → %1").arg(effective));
    }
}
