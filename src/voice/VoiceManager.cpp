// ============================================================
// VoiceManager.cpp - 语音管理器实现
// ============================================================

#include "PlatformDefines.h"
#include "voice/VoiceManager.h"
#include "core/EventBus.h"

#ifdef HAS_TEXT_TO_SPEECH
#include <QTextToSpeech>
#endif

#include <QJsonObject>
#include <QVariant>

namespace petapp {
namespace voice {

VoiceManager::VoiceManager(core::EventBus& eventBus, QObject* parent)
    : QObject(parent)
    , m_eventBus(eventBus)
    , m_tts(nullptr)
    , m_available(false)
{
    // 订阅语音指令事件
    m_eventBus.on("voice_command", [this](const QVariant& variant) {
        onVoiceCommand(variant);
    });
}

VoiceManager::~VoiceManager()
{
    stop();
}

void VoiceManager::initialize()
{
#ifdef HAS_TEXT_TO_SPEECH
    // 尝试创建 QTextToSpeech 实例
    m_tts = new QTextToSpeech(this);

    if (m_tts->state() == QTextToSpeech::Error) {
        // TTS 不可用，优雅降级
        delete m_tts;
        m_tts = nullptr;
        m_available = false;
    } else {
        m_available = true;
        // 设置默认参数
        m_tts->setRate(0.0);
        m_tts->setVolume(0.8);

        // 连接状态信号
        connect(m_tts, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
            if (state == QTextToSpeech::Speaking) {
                Q_EMIT speakingStarted();
            } else if (state == QTextToSpeech::Ready) {
                Q_EMIT speakingFinished();
            }
        });
    }
#else
    // TTS 组件不可用，优雅降级
    m_tts = nullptr;
    m_available = false;
#endif
}

void VoiceManager::speak(const QString& text)
{
#ifdef HAS_TEXT_TO_SPEECH
    if (!m_available || !m_tts) {
        return;
    }
    m_tts->say(text);
#else
    Q_UNUSED(text);
#endif
}

void VoiceManager::stop()
{
#ifdef HAS_TEXT_TO_SPEECH
    if (m_tts && m_available) {
        m_tts->stop();
    }
#endif
}

void VoiceManager::setRate(double rate)
{
#ifdef HAS_TEXT_TO_SPEECH
    if (m_tts && m_available) {
        m_tts->setRate(rate);
    }
#else
    Q_UNUSED(rate);
#endif
}

void VoiceManager::setVolume(double volume)
{
#ifdef HAS_TEXT_TO_SPEECH
    if (m_tts && m_available) {
        m_tts->setVolume(volume);
    }
#else
    Q_UNUSED(volume);
#endif
}

bool VoiceManager::isAvailable() const
{
    return m_available;
}

void VoiceManager::onVoiceCommand(const QVariant& variant)
{
    QJsonObject cmd = variant.toJsonObject();
    QString action = cmd.value("action").toString();

    if (action == "speak") {
        speak(cmd.value("text").toString());
    } else if (action == "stop") {
        stop();
    } else if (action == "set_rate") {
        setRate(cmd.value("rate").toDouble(0.0));
    } else if (action == "set_volume") {
        setVolume(cmd.value("volume").toDouble(0.8));
    }
}

} // namespace voice
} // namespace petapp