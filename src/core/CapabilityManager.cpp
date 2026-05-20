// ============================================================
// CapabilityManager.cpp - 能力管理器实现
// ============================================================

#include "PlatformDefines.h"
#include "core/CapabilityManager.h"
#include "core/EventBus.h"

namespace petapp {
namespace core {

CapabilityManager::CapabilityManager(EventBus& eventBus, QObject* parent)
    : QObject(parent)
    , m_eventBus(eventBus)
{
}

CapabilityManager::~CapabilityManager() = default;

void CapabilityManager::initialize()
{
    // 初始发布状态
    publishStatus();
}

void CapabilityManager::setManualOffline(bool offline)
{
    if (m_manualOffline != offline) {
        m_manualOffline = offline;
        publishStatus();
    }
}

bool CapabilityManager::isManualOffline() const
{
    return m_manualOffline;
}

bool CapabilityManager::isOnline() const
{
    return !m_manualOffline && m_networkReachable;
}

void CapabilityManager::setOnline(bool online)
{
    m_manualOffline = !online;
    publishStatus();
}

void CapabilityManager::setOffline()
{
    m_manualOffline = true;
    publishStatus();
}

bool CapabilityManager::isNetworkAvailable() const
{
    return !m_manualOffline && m_networkReachable;
}

bool CapabilityManager::canRunSkill(bool requiresNetwork) const
{
    if (requiresNetwork) {
        return isNetworkAvailable();
    }
    return true;
}

void CapabilityManager::onNetworkStatusChanged(bool online)
{
    m_networkReachable = online;
    publishStatus();
}

void CapabilityManager::publishStatus()
{
    bool online = isNetworkAvailable();
    emit onlineStatusChanged(online);
    m_eventBus.emitEvent("online_status_changed", online);
}

} // namespace core
} // namespace petapp