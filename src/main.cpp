// ============================================================
// main.cpp - PetApp 桌宠系统入口
// ============================================================

#include "PlatformDefines.h"

#include <QApplication>
#include "core/CrashHandler.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PetApp");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PetApp");

    // 初始化崩溃处理（需要在 QApplication 之后，因为需要 applicationDirPath）
    petapp::core::CrashHandler::initialize();

    // 创建主窗口（启动时隐藏，只显示托盘图标）
    petapp::ui::MainWindow mainWindow;
    // mainWindow.show();  // 启动时不显示窗口，只显示托盘

    return app.exec();
}