// ============================================================
// SkillManager.cpp - 技能管理器实现
// ============================================================

#include "PlatformDefines.h"
#include "core/SkillManager.h"
#include "core/EventBus.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

namespace petapp {
namespace core {

SkillManager::SkillManager(EventBus& eventBus, QObject* parent)
    : QObject(parent)
    , m_eventBus(eventBus)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_activeProcess(nullptr)
{
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &SkillManager::onDirectoryChanged);
}

SkillManager::~SkillManager()
{
    if (m_activeProcess) {
        if (m_activeProcess->state() != QProcess::NotRunning) {
            m_activeProcess->kill();
            m_activeProcess->waitForFinished(1000);
        }
    }
}

void SkillManager::initialize()
{
    // 确保 plugins/ 目录存在
    QString appDir = QCoreApplication::applicationDirPath();
    QString pluginsDir = appDir + "/plugins";
    QDir dir(pluginsDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 监视 plugins/ 目录
    m_fileWatcher->addPath(pluginsDir);

    // 扫描现有技能
    scanSkills();
}

void SkillManager::scanSkills()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString pluginsDir = appDir + "/plugins";
    QDir dir(pluginsDir);

    m_skills.clear();

    // 查找所有 .exe 文件
    QStringList exeFiles = dir.entryList({"*.exe"}, QDir::Files);
    for (const QString& exeName : exeFiles) {
        QString baseName = QFileInfo(exeName).completeBaseName();
        QString manifestPath = dir.absoluteFilePath(baseName + ".manifest.json");

        // 尝试读取同名的 .manifest.json
        if (QFile::exists(manifestPath)) {
            SkillInfo info = loadManifest(manifestPath);
            info.exePath = dir.absoluteFilePath(exeName);
            m_skills.insert(info.id, info);
        } else {
            // 也尝试读取同名的 manifest.json（不带技能名前缀）
            QString altManifestPath = dir.absoluteFilePath("manifest.json");
            if (QFile::exists(altManifestPath)) {
                SkillInfo info = loadManifest(altManifestPath);
                info.executable = exeName;
                info.exePath = dir.absoluteFilePath(exeName);
                m_skills.insert(info.id, info);
            }
        }
    }

    emit skillsUpdated();
}

SkillInfo SkillManager::loadManifest(const QString& manifestPath)
{
    SkillInfo info;
    info.manifestPath = manifestPath;

    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return info;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return info;
    }

    QJsonObject obj = doc.object();
    info.id = obj.value("id").toString();
    info.name = obj.value("name").toString();
    info.version = obj.value("version").toString("1.0.0");
    info.requiresNetwork = obj.value("requires_network").toBool(false);
    info.resourceWeight = obj.value("resource_weight").toInt(1);
    info.executable = obj.value("executable").toString();
    info.description = obj.value("description").toString();

    return info;
}

bool SkillManager::runSkill(const QString& skillId,
                            const QJsonObject& params,
                            std::function<void(const QJsonObject&)> callback)
{
    if (!m_skills.contains(skillId)) {
        qWarning() << "Skill not found:" << skillId;
        return false;
    }

    if (m_activeProcess && m_activeProcess->state() != QProcess::NotRunning) {
        qWarning() << "Another skill is already running";
        return false;
    }

    const SkillInfo& info = m_skills[skillId];

    // 创建并启动进程
    m_activeProcess = new QProcess(this);
    m_activeSkillId = skillId;
    m_activeCallback = callback;

    connect(m_activeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SkillManager::onProcessFinished);

    // 构建 JSON 请求
    QJsonObject request;
    request["method"] = "execute";
    request["id"] = 1;
    request["params"] = params;

    QJsonDocument requestDoc(request);
    QByteArray requestData = requestDoc.toJson(QJsonDocument::Compact) + "\n";

    // 启动进程
    m_activeProcess->start(info.exePath, QStringList());
    if (!m_activeProcess->waitForStarted(3000)) {
        qWarning() << "Failed to start skill:" << info.executable;
        cleanupProcess();
        return false;
    }

    // 写入请求
    m_activeProcess->write(requestData);
    m_activeProcess->closeWriteChannel();

    // 设置超时
    QTimer::singleShot(m_timeoutMs, this, [this]() {
        if (m_activeProcess && m_activeProcess->state() != QProcess::NotRunning) {
            qWarning() << "Skill timeout, killing:" << m_activeSkillId;
            m_activeProcess->kill();
        }
    });

    return true;
}

void SkillManager::onProcessFinished()
{
    if (!m_activeProcess) {
        return;
    }

    QByteArray output = m_activeProcess->readAllStandardOutput();
    QJsonObject result = parseProcessOutput(output);

    bool success = result.value("error").isNull();
    if (m_activeCallback) {
        m_activeCallback(result);
    }

    emit skillCompleted(m_activeSkillId, result, success);

    cleanupProcess();
}

void SkillManager::onDirectoryChanged(const QString& path)
{
    Q_UNUSED(path);
    scanSkills();
}

QMap<QString, SkillInfo> SkillManager::availableSkills() const
{
    return m_skills;
}

SkillInfo SkillManager::skillInfo(const QString& skillId) const
{
    if (m_skills.contains(skillId)) {
        return m_skills[skillId];
    }
    return SkillInfo();
}

bool SkillManager::hasSkill(const QString& skillId) const
{
    return m_skills.contains(skillId);
}

QJsonObject SkillManager::parseProcessOutput(const QByteArray& output)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(output.trimmed(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject errorResult;
        errorResult["id"] = -1;
        errorResult["result"] = QJsonValue::Null;
        QJsonObject error;
        error["code"] = -1;
        error["message"] = "Failed to parse skill output: " + QString::fromUtf8(output.trimmed());
        errorResult["error"] = error;
        return errorResult;
    }
    return doc.object();
}

void SkillManager::cleanupProcess()
{
    if (m_activeProcess) {
        m_activeProcess->deleteLater();
        m_activeProcess = nullptr;
    }
    m_activeSkillId.clear();
    m_activeCallback = nullptr;
}

bool SkillManager::canRunSkill(const QString& skillId, bool isNetworkAvailable) const
{
    if (!m_skills.contains(skillId)) {
        return false;
    }

    const SkillInfo& info = m_skills[skillId];

    // 如果技能需要网络但网络不可用，则不可运行
    if (info.requiresNetwork && !isNetworkAvailable) {
        return false;
    }

    return true;
}

QStringList SkillManager::skillsRequiringNetwork() const
{
    QStringList result;
    for (auto it = m_skills.constBegin(); it != m_skills.constEnd(); ++it) {
        if (it.value().requiresNetwork) {
            result.append(it.key());
        }
    }
    return result;
}

} // namespace core
} // namespace petapp