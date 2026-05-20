// ============================================================
// EmotionModule.cpp - 情感模块实现
// ============================================================

#include "PlatformDefines.h"
#include "core/EmotionModule.h"
#include "core/EventBus.h"

#include <algorithm>

namespace petapp {
namespace core {

EmotionModule::EmotionModule(EventBus& eventBus, QObject* parent)
    : QObject(parent)
    , m_eventBus(eventBus)
{
    // 注册 EmotionState 元类型
    qRegisterMetaType<EmotionState>("EmotionState");
}

EmotionModule::~EmotionModule() = default;

void EmotionModule::modify(double pleasureDelta, double curiosityDelta, double tirednessDelta)
{
    m_state.pleasure = clamp(m_state.pleasure + pleasureDelta);
    m_state.curiosity = clamp(m_state.curiosity + curiosityDelta);
    m_state.tiredness = clamp(m_state.tiredness + tirednessDelta);
    publishEmotion();
}

void EmotionModule::setEmotion(double pleasure, double curiosity, double tiredness)
{
    m_state.pleasure = clamp(pleasure);
    m_state.curiosity = clamp(curiosity);
    m_state.tiredness = clamp(tiredness);
    publishEmotion();
}

void EmotionModule::decay(double amount)
{
    // 愉悦和好奇自然衰减，疲倦自然增加
    m_state.pleasure = clamp(m_state.pleasure - amount);
    m_state.curiosity = clamp(m_state.curiosity - amount);
    m_state.tiredness = clamp(m_state.tiredness + amount);
    publishEmotion();
}

EmotionState EmotionModule::currentEmotion() const
{
    return m_state;
}

double EmotionModule::clamp(double value)
{
    return std::max(0.0, std::min(1.0, value));
}

void EmotionModule::publishEmotion()
{
    emit emotionChanged(m_state);
    m_eventBus.emitEvent("emotion_changed", QVariant::fromValue(m_state));
}

} // namespace core
} // namespace petapp