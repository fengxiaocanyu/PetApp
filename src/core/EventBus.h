#ifndef PETAPP_CORE_EVENTBUS_H
#define PETAPP_CORE_EVENTBUS_H

// ============================================================
// EventBus.h - 全局事件总线（单例模式）
// 模块间通过事件发布/订阅进行通信，禁止直接函数调用跨模块
// ============================================================

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>
#include <functional>

namespace petapp {
namespace core {

/**
 * @brief 事件总线 - 全局单例，模块间发布/订阅事件
 *
 * 使用方式：
 *   EventBus::instance().on("event_name", callback);
 *   EventBus::instance().emitEvent("event_name", data);
 */
class EventBus : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取全局单例实例
     * @return EventBus& 单例引用
     */
    static EventBus& instance();

    /**
     * @brief 析构函数
     */
    ~EventBus() override;

    // 禁止拷贝和赋值
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /**
     * @brief 订阅事件
     * @param eventName 事件名称
     * @param callback 回调函数，接收 QVariant 数据
     * @return int 回调 ID，可用于取消订阅
     */
    int on(const QString& eventName, std::function<void(const QVariant&)> callback);

    /**
     * @brief 取消订阅
     * @param callbackId 回调 ID（从 on() 返回）
     */
    void off(int callbackId);

    /**
     * @brief 发布事件
     * @param eventName 事件名称
     * @param data 事件数据
     */
    void emitEvent(const QString& eventName, const QVariant& data = QVariant());

Q_SIGNALS:
    /**
     * @brief Qt 信号 - 用于连接 QObject 槽函数
     * @param eventName 事件名称
     * @param data 事件数据
     */
    void eventTriggered(const QString& eventName, const QVariant& data);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit EventBus(QObject* parent = nullptr);

    struct CallbackEntry {
        int id;
        QString eventName;
        std::function<void(const QVariant&)> callback;
    };

    QMap<int, CallbackEntry> m_callbacks;
    int m_nextId = 1;
};

} // namespace core
} // namespace petapp

#endif // PETAPP_CORE_EVENTBUS_H