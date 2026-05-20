// ============================================================
// MemoryModule.cpp - 持久化 KV 存储模块实现
// ============================================================

#include "PlatformDefines.h"
#include "core/MemoryModule.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>

namespace petapp {
namespace core {

MemoryModule::MemoryModule(QObject* parent)
    : QObject(parent)
{
    // 数据文件路径：可执行文件所在目录的 data/memory/memory.json
    QString appDir = QCoreApplication::applicationDirPath();
    m_filePath = appDir + "/data/memory/memory.json";
}

MemoryModule::~MemoryModule() = default;

bool MemoryModule::load()
{
    QFile file(m_filePath);
    if (!file.exists()) {
        // 文件不存在不是错误，返回空数据
        m_data = QJsonObject();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray rawData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }

    if (!doc.isObject()) {
        return false;
    }

    m_data = doc.object();
    emit dataLoaded();
    return true;
}

bool MemoryModule::save()
{
    // 确保目录存在
    QFileInfo fileInfo(m_filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QJsonDocument doc(m_data);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    emit dataSaved();
    return true;
}

void MemoryModule::setValue(const QString& key, const QVariant& value)
{
    m_data.insert(key, QJsonValue::fromVariant(value));
}

QVariant MemoryModule::getValue(const QString& key, const QVariant& defaultValue) const
{
    if (m_data.contains(key)) {
        return m_data.value(key).toVariant();
    }
    return defaultValue;
}

bool MemoryModule::hasKey(const QString& key) const
{
    return m_data.contains(key);
}

void MemoryModule::removeKey(const QString& key)
{
    m_data.remove(key);
}

QString MemoryModule::dataFilePath() const
{
    return m_filePath;
}

} // namespace core
} // namespace petapp