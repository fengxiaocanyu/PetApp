// ============================================================
// HeartbeatModule.cpp - 心跳模块实现
// ============================================================

#include "PlatformDefines.h"
#include "core/HeartbeatModule.h"
#include "core/EventBus.h"

#include <QTimer>

namespace petapp {
namespace core {

HeartbeatModule::HeartbeatModule(EventBus& eventBus,
                                 int intervalMs,
                                 QObject* parent)
    : QObject(parent)
    , m_eventBus(eventBus)
    , m_timer(new QTimer(this))
    , m_intervalMs(intervalMs)
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &HeartbeatModule::onTimerTick);
}

HeartbeatModule::~HeartbeatModule()
{
    stop();
}

void HeartbeatModule::start()
{
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void HeartbeatModule::stop()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
}

void HeartbeatModule::setInterval(int intervalMs)
{
    m_intervalMs = intervalMs;
    m_timer->setInterval(m_intervalMs);
}

int HeartbeatModule::interval() const
{
    return m_intervalMs;
}

bool HeartbeatModule::isRunning() const
{
    return m_timer->isActive();
}

void HeartbeatModule::onTimerTick()
{
    m_eventBus.emitEvent("heartbeat", QVariant());
}

} // namespace core
} // namespace petapp