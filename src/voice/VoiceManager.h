#ifndef PETAPP_VOICE_VOICEMANAGER_H
#define PETAPP_VOICE_VOICEMANAGER_H

// ============================================================
// VoiceManager.h - 语音管理器
// 使用 QTextToSpeech 实现 TTS，支持语速、音量调节
// ============================================================

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QTextToSpeech;
QT_END_NAMESPACE

namespace petapp { namespace core {
    class EventBus;
}}

namespace petapp {
namespace voice {

/**
 * @brief 语音管理器 - 文本转语音
 *
 * 使用 QTextToSpeech（Windows SAPI）实现 TTS。
 * 支持语速、音量调节。
 * 如果 QTextToSpeech 不可用，优雅降级（不崩溃、不报错）。
 */
class VoiceManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param parent 父对象
     */
    explicit VoiceManager(core::EventBus& eventBus, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~VoiceManager() override;

    /**
     * @brief 初始化
     */
    void initialize();

    /**
     * @brief 语音播报
     * @param text 要播报的文本
     */
    void speak(const QString& text);

    /**
     * @brief 停止播报
     */
    void stop();

    /**
     * @brief 设置语速
     * @param rate 语速值（-1.0 到 1.0，0 为正常）
     */
    void setRate(double rate);

    /**
     * @brief 设置音量
     * @param volume 音量值（0.0 到 1.0）
     */
    void setVolume(double volume);

    /**
     * @brief 检查 TTS 是否可用
     * @return true 如果 TTS 可用
     */
    bool isAvailable() const;

Q_SIGNALS:
    /**
     * @brief 语音播报开始信号
     */
    void speakingStarted();

    /**
     * @brief 语音播报结束信号
     */
    void speakingFinished();

private Q_SLOTS:
    /**
     * @brief 处理语音指令事件
     * @param variant 指令数据
     */
    void onVoiceCommand(const QVariant& variant);

private:
    core::EventBus& m_eventBus;
    QTextToSpeech* m_tts;
    bool m_available;
};

} // namespace voice
} // namespace petapp

#endif // PETAPP_VOICE_VOICEMANAGER_H