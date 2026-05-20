#ifndef PETAPP_CORE_CRASHHANDLER_H
#define PETAPP_CORE_CRASHHANDLER_H

// ============================================================
// CrashHandler.h - 崩溃处理模块
// 捕获未处理异常和 Qt 消息，生成 minidump 至 logs/crash/
// ============================================================

#include <QString>

namespace petapp {
namespace core {

/**
 * @brief 崩溃处理模块
 *
 * 使用 qInstallMessageHandler 捕获 Qt 消息，
 * 使用 SetUnhandledExceptionFilter 捕获原生异常，
 * 生成崩溃转储文件至 logs/crash/ 目录。
 */
class CrashHandler
{
public:
    /**
     * @brief 初始化崩溃处理
     * 应在 main() 函数开头调用
     */
    static void initialize();

    /**
     * @brief 获取崩溃日志目录
     * @return 崩溃日志目录路径
     */
    static QString crashDirectory();

private:
    /**
     * @brief Qt 消息处理回调
     */
    static void messageHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& msg);

    /**
     * @brief 写入崩溃转储文件
     * @param crashDir 崩溃日志目录
     * @param type 消息类型
     * @param msg 错误消息
     */
    static void writeCrashDump(const QString& crashDir,
                               QtMsgType type,
                               const QString& msg);

    static bool s_initialized;
};

} // namespace core
} // namespace petapp

#endif // PETAPP_CORE_CRASHHANDLER_H