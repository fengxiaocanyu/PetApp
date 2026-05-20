// ============================================================
// CrashHandler.cpp - 崩溃处理模块实现
// ============================================================

#include "PlatformDefines.h"
#include "core/CrashHandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

// Windows 特定头（仅在 .cpp 中）
#ifdef Q_OS_WIN
#include <windows.h>
#include <dbghelp.h>
#include <signal.h>
#endif

namespace petapp {
namespace core {

bool CrashHandler::s_initialized = false;

void CrashHandler::initialize()
{
    if (s_initialized) {
        return;
    }
    s_initialized = true;

    // 确保崩溃日志目录存在
    QString crashDir = crashDirectory();
    QDir dir(crashDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 安装 Qt 消息处理器
    qInstallMessageHandler(messageHandler);

#ifdef Q_OS_WIN
    // 设置未处理异常过滤器
    SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* exceptionInfo) -> LONG {
        // 生成 minidump
        QString crashDir = crashDirectory();
        QString dumpPath = crashDir + "/crash_" +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".dmp";

        HANDLE hFile = CreateFileA(
            dumpPath.toLocal8Bit().constData(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
            dumpInfo.ThreadId = GetCurrentThreadId();
            dumpInfo.ExceptionPointers = exceptionInfo;
            dumpInfo.ClientPointers = FALSE;

            MiniDumpWriteDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                hFile,
                MiniDumpNormal,
                &dumpInfo,
                nullptr,
                nullptr);

            CloseHandle(hFile);
        }

        // 记录错误日志
        QFile logFile(crashDir + "/crash.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&logFile);
            stream << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
                   << "] Unhandled exception at address: 0x"
                   << QString::number(reinterpret_cast<quintptr>(exceptionInfo->ExceptionRecord->ExceptionAddress), 16)
                   << ", code: 0x"
                   << QString::number(exceptionInfo->ExceptionRecord->ExceptionCode, 16)
                   << "\n";
            logFile.close();
        }

        return EXCEPTION_EXECUTE_HANDLER;
    });

    // 设置信号处理
    signal(SIGABRT, [](int) {
        // 触发异常过滤器
        raise(SIGABRT);
    });
#endif

    qInfo() << "CrashHandler initialized, crash directory:" << crashDir;
}

QString CrashHandler::crashDirectory()
{
    QString appDir = QCoreApplication::applicationDirPath();
    return appDir + "/logs/crash";
}

void CrashHandler::messageHandler(QtMsgType type,
                                  const QMessageLogContext& context,
                                  const QString& msg)
{
    // 获取崩溃日志目录
    QString crashDir = crashDirectory();

    // 写入日志文件
    QFile logFile(crashDir + "/app.log");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&logFile);
        QString level;
        switch (type) {
            case QtDebugMsg:    level = "DEBUG"; break;
            case QtInfoMsg:     level = "INFO"; break;
            case QtWarningMsg:  level = "WARN"; break;
            case QtCriticalMsg: level = "CRITICAL"; break;
            case QtFatalMsg:    level = "FATAL"; break;
        }

        stream << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
               << "] [" << level << "] "
               << msg;

        if (context.file && context.line > 0) {
            stream << " (" << context.file << ":" << context.line << ")";
        }
        stream << "\n";
        logFile.close();
    }

    // 对于致命错误，生成崩溃转储
    if (type == QtFatalMsg) {
        writeCrashDump(crashDir, type, msg);
    }

    // 同时输出到 stderr
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "DEBUG: %s\n", localMsg.constData());
            break;
        case QtInfoMsg:
            fprintf(stderr, "INFO: %s\n", localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "WARN: %s\n", localMsg.constData());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "CRITICAL: %s\n", localMsg.constData());
            break;
        case QtFatalMsg:
            fprintf(stderr, "FATAL: %s\n", localMsg.constData());
            break;
    }

    if (type == QtFatalMsg) {
        // 中止
        abort();
    }
}

void CrashHandler::writeCrashDump(const QString& crashDir,
                                  QtMsgType type,
                                  const QString& msg)
{
    QString dumpPath = crashDir + "/fatal_" +
        QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".log";

    QFile file(dumpPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "Fatal Error Report\n";
        stream << "==================\n";
        stream << "Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz") << "\n";
        stream << "Type: " << (type == QtFatalMsg ? "FATAL" : "CRITICAL") << "\n";
        stream << "Message: " << msg << "\n";
        stream << "Application: " << QCoreApplication::applicationName() << "\n";
        stream << "Version: " << QCoreApplication::applicationVersion() << "\n";
        stream << "PID: " << QCoreApplication::applicationPid() << "\n";
        file.close();
    }
}

} // namespace core
} // namespace petapp