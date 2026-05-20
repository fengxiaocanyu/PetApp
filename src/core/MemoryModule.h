#ifndef PETAPP_CORE_MEMORYMODULE_H
#define PETAPP_CORE_MEMORYMODULE_H

// ============================================================
// MemoryModule.h - 持久化 KV 存储模块
// 基于 QJsonDocument 读写 data/memory/memory.json
// ============================================================

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>

namespace petapp {
namespace core {

/**
 * @brief 内存/持久化模块 - 提供键值存储功能
 *
 * 数据存储在 data/memory/memory.json 文件中。
 * 提供 save()、load()、setValue()、getValue() 接口。
 */
class MemoryModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MemoryModule(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MemoryModule() override;

    /**
     * @brief 从文件加载数据
     * @return true 加载成功，false 加载失败
     */
    bool load();

    /**
     * @brief 保存数据到文件
     * @return true 保存成功，false 保存失败
     */
    bool save();

    /**
     * @brief 设置键值
     * @param key 键名
     * @param value 值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 获取值
     * @param key 键名
     * @param defaultValue 默认值（如果键不存在）
     * @return QVariant 存储的值或默认值
     */
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 检查键是否存在
     * @param key 键名
     * @return true 如果键存在
     */
    bool hasKey(const QString& key) const;

    /**
     * @brief 删除键
     * @param key 键名
     */
    void removeKey(const QString& key);

    /**
     * @brief 获取数据文件路径
     * @return QString 数据文件路径
     */
    QString dataFilePath() const;

Q_SIGNALS:
    /**
     * @brief 数据已保存信号
     */
    void dataSaved();

    /**
     * @brief 数据已加载信号
     */
    void dataLoaded();

private:
    QJsonObject m_data;
    QString m_filePath;
};

} // namespace core
} // namespace petapp

#endif // PETAPP_CORE_MEMORYMODULE_H