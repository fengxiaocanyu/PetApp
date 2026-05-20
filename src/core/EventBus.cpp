// ============================================================
// EventBus.cpp - 全局事件总线实现
// ============================================================

#include "PlatformDefines.h"
#include "core/EventBus.h"

namespace petapp {
namespace core {

EventBus::EventBus(QObject* parent)
    : QObject(parent)
{
}

EventBus::~EventBus() = default;

EventBus& EventBus::instance()
{
    static EventBus s_instance;
    return s_instance;
}

int EventBus::on(const QString& eventName, std::function<void(const QVariant&)> callback)
{
    int id = m_nextId++;
    m_callbacks.insert(id, { id, eventName, callback });
    return id;
}

void EventBus::off(int callbackId)
{
    m_callbacks.remove(callbackId);
}

void EventBus::emitEvent(const QString& eventName, const QVariant& data)
{
    // 通过 Qt 信号触发
    emit eventTriggered(eventName, data);

    // 遍历回调列表，触发匹配的回调
    for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it) {
        if (it.value().eventName == eventName) {
            it.value().callback(data);
        }
    }
}

} // namespace core
} // namespace petapp