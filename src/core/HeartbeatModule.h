#ifndef PETAPP_CORE_HEARTBEATMODULE_H
#define PETAPP_CORE_HEARTBEATMODULE_H

// ============================================================
// HeartbeatModule.h - 心跳模块
// 基于 QTimer，默认 10 秒触发一次心跳事件
// ============================================================

#include <QObject>

namespace petapp { namespace core {
    class EventBus;
}}

namespace petapp {
namespace core {

/**
 * @brief 心跳模块 - 定时触发心跳事件
 *
 * 默认间隔 10 秒，通过 EventBus 发布 "heartbeat" 事件。
 * 用于驱动情感衰减、后台任务等。
 */
class HeartbeatModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param intervalMs 心跳间隔（毫秒），默认 10000
     * @param parent 父对象
     */
    explicit HeartbeatModule(EventBus& eventBus,
                             int intervalMs = 10000,
                             QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~HeartbeatModule() override;

    /**
     * @brief 启动心跳
     */
    void start();

    /**
     * @brief 停止心跳
     */
    void stop();

    /**
     * @brief 设置心跳间隔
     * @param intervalMs 间隔毫秒数
     */
    void setInterval(int intervalMs);

    /**
     * @brief 获取当前心跳间隔
     * @return int 间隔毫秒数
     */
    int interval() const;

    /**
     * @brief 检查心跳是否在运行
     * @return true 如果正在运行
     */
    bool isRunning() const;

private Q_SLOTS:
    /**
     * @brief 定时器触发槽函数
     */
    void onTimerTick();

private:
    EventBus& m_eventBus;
    class QTimer* m_timer;
    int m_intervalMs;
};

} // namespace core
} // namespace petapp

#endif // PETAPP_CORE_HEARTBEATMODULE_H