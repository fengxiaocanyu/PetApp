#ifndef PETAPP_CORE_CAPABILITYMANAGER_H
#define PETAPP_CORE_CAPABILITYMANAGER_H

// ============================================================
// CapabilityManager.h - 能力管理器
// 监听网络状态，提供手动离线模式开关
// ============================================================

#include <QObject>
#include <QString>
#include <QVariant>

namespace petapp { namespace core {
    class EventBus;
}}

namespace petapp {
namespace core {

/**
 * @brief 能力管理器 - 管理网络状态和技能可用性
 *
 * 监听网络状态，提供手动离线模式开关。
 * 通过 EventBus 发布 "online_status_changed" 事件。
 */
class CapabilityManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param parent 父对象
     */
    explicit CapabilityManager(EventBus& eventBus, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~CapabilityManager() override;

    /**
     * @brief 初始化（开始监听网络状态）
     */
    void initialize();

    /**
     * @brief 设置手动离线模式
     * @param offline true 强制离线，false 自动检测
     */
    void setManualOffline(bool offline);

    /**
     * @brief 检查是否手动离线
     * @return true 如果手动离线
     */
    bool isManualOffline() const;

    /**
     * @brief 检查是否在线（便捷方法）
     * @return true 如果在线
     */
    bool isOnline() const;

    /**
     * @brief 设置在线状态
     * @param online true 设为在线，false 设为离线
     */
    void setOnline(bool online);

    /**
     * @brief 强制离线
     */
    void setOffline();

    /**
     * @brief 检查网络是否可用
     * @return true 如果网络可用（非手动离线且网络可达）
     */
    bool isNetworkAvailable() const;

    /**
     * @brief 检查技能是否可运行
     * @param requiresNetwork 技能是否需要网络
     * @return true 如果技能可运行
     */
    bool canRunSkill(bool requiresNetwork) const;

Q_SIGNALS:
    /**
     * @brief 在线状态变化信号
     * @param online 是否在线
     */
    void onlineStatusChanged(bool online);

private Q_SLOTS:
    /**
     * @brief 网络状态变化槽函数
     * @param online 是否在线
     */
    void onNetworkStatusChanged(bool online);

private:
    /**
     * @brief 发布在线状态事件
     */
    void publishStatus();

    EventBus& m_eventBus;
    bool m_manualOffline = true;  // 默认离线
    bool m_networkReachable = false;
};

} // namespace core
} // namespace petapp

#endif // PETAPP_CORE_CAPABILITYMANAGER_H