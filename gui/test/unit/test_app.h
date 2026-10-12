// test/unit/test_app.h - 测试用 QApplication 环境（gtest main 入口）
#pragma once

#include <QApplication>
#include <QLocale>
#include <gtest/gtest.h>

// 刻意泄漏 QApplication：静态 Qt 下若在退出时析构会与静态插件/线程池析构顺序冲突
class QtEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        static int argc = 1;
        static char arg0[] = "PsigMatcher_test";
        static char* argv[] = {arg0, nullptr};
        m_app = new QApplication(argc, argv); // 不 delete（刻意泄漏）
        // 固定默认语言为中文，使断言与运行机器的系统语言无关（install 默认语言经 QLocale() 判定）
        QLocale::setDefault(QLocale(QStringLiteral("zh_CN")));
    }
    void TearDown() override {}
private:
    QApplication* m_app = nullptr;
};

inline void psigRegisterQtEnv() {
    static const bool registered = []() {
        ::testing::AddGlobalTestEnvironment(new QtEnvironment());
        return true;
    }();
    (void)registered;
}

#define PSIG_REGISTER_QT_ENV() namespace { const bool g_psig_qt_env_ = (psigRegisterQtEnv(), true); }
