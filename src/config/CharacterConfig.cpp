// ============================================================
// CharacterConfig.cpp - 角色参数配置模块实现
// ============================================================

#include "PlatformDefines.h"
#include "config/CharacterConfig.h"
#include "core/EventBus.h"
#include "core/MemoryModule.h"

#include <QJsonObject>

namespace petapp {
namespace config {

CharacterConfig::CharacterConfig(core::EventBus& eventBus,
                                 core::MemoryModule& memoryModule,
                                 QObject* parent)
    : QObject(parent)
    , m_eventBus(eventBus)
    , m_memoryModule(memoryModule)
{
}

CharacterConfig::~CharacterConfig() = default;

void CharacterConfig::initialize()
{
    // 从存储加载配置
    QVariant scaleVal = m_memoryModule.getValue("character_scale");
    if (scaleVal.isValid()) {
        m_params.scale = scaleVal.toDouble();
    }

    QVariant offsetXVal = m_memoryModule.getValue("character_offset_x");
    if (offsetXVal.isValid()) {
        m_params.offsetX = offsetXVal.toInt();
    }

    QVariant offsetYVal = m_memoryModule.getValue("character_offset_y");
    if (offsetYVal.isValid()) {
        m_params.offsetY = offsetYVal.toInt();
    }

    QVariant animIntervalVal = m_memoryModule.getValue("character_animation_interval");
    if (animIntervalVal.isValid()) {
        m_params.animationInterval = animIntervalVal.toInt();
    }

    QVariant showBubbleVal = m_memoryModule.getValue("character_show_bubble");
    if (showBubbleVal.isValid()) {
        m_params.showBubble = showBubbleVal.toBool();
    }

    // 发布初始配置
    emitConfigChanged();
}

void CharacterConfig::setScale(double scale)
{
    if (scale < 0.1) scale = 0.1;
    if (scale > 3.0) scale = 3.0;
    m_params.scale = scale;
    saveConfig();
    emitConfigChanged();
}

void CharacterConfig::setOffsetX(int offsetX)
{
    m_params.offsetX = offsetX;
    saveConfig();
    emitConfigChanged();
}

void CharacterConfig::setOffsetY(int offsetY)
{
    m_params.offsetY = offsetY;
    saveConfig();
    emitConfigChanged();
}

void CharacterConfig::setAnimationInterval(int ms)
{
    if (ms < 50) ms = 50;
    if (ms > 2000) ms = 2000;
    m_params.animationInterval = ms;
    saveConfig();
    emitConfigChanged();
}

void CharacterConfig::setShowBubble(bool show)
{
    m_params.showBubble = show;
    saveConfig();
    emitConfigChanged();
}

void CharacterConfig::saveConfig()
{
    m_memoryModule.setValue("character_scale", m_params.scale);
    m_memoryModule.setValue("character_offset_x", m_params.offsetX);
    m_memoryModule.setValue("character_offset_y", m_params.offsetY);
    m_memoryModule.setValue("character_animation_interval", m_params.animationInterval);
    m_memoryModule.setValue("character_show_bubble", m_params.showBubble);
    m_memoryModule.save();
}

void CharacterConfig::emitConfigChanged()
{
    // 通过 EventBus 发布配置变更事件
    QJsonObject data;
    data["scale"] = m_params.scale;
    data["offsetX"] = m_params.offsetX;
    data["offsetY"] = m_params.offsetY;
    data["animationInterval"] = m_params.animationInterval;
    data["showBubble"] = m_params.showBubble;
    m_eventBus.emitEvent("character_config_changed", QVariant(data));

    Q_EMIT configChanged(m_params);
}

} // namespace config
} // namespace petapp