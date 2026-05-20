#ifndef PETAPP_CORE_EMOTIONMODULE_H
#define PETAPP_CORE_EMOTIONMODULE_H

// ============================================================
// EmotionModule.h - 情感模块
// 维护情感向量（愉悦、好奇、疲倦），范围 0-1
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
 * @brief 情感状态结构体
 */
struct EmotionState {
    double pleasure = 0.5;    ///< 愉悦度 (0-1)
    double curiosity = 0.5;   ///< 好奇度 (0-1)
    double tiredness = 0.5;   ///< 疲倦度 (0-1)
};

/**
 * @brief 情感模块 - 维护情感向量
 *
 * 每心跳自然衰减 0.02，外部事件可增减。
 * 通过 EventBus 发布 "emotion_changed" 事件。
 */
class EmotionModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param parent 父对象
     */
    explicit EmotionModule(EventBus& eventBus, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~EmotionModule() override;

    /**
     * @brief 修改情感值
     * @param pleasureDelta 愉悦度变化量 (-1.0 ~ 1.0)
     * @param curiosityDelta 好奇度变化量 (-1.0 ~ 1.0)
     * @param tirednessDelta 疲倦度变化量 (-1.0 ~ 1.0)
     */
    void modify(double pleasureDelta, double curiosityDelta, double tirednessDelta);

    /**
     * @brief 直接设置情感值
     * @param pleasure 愉悦度 (0-1)
     * @param curiosity 好奇度 (0-1)
     * @param tiredness 疲倦度 (0-1)
     */
    void setEmotion(double pleasure, double curiosity, double tiredness);

    /**
     * @brief 情感自然衰减（每心跳调用）
     * @param decay 衰减量，默认 0.02
     */
    void decay(double amount = 0.02);

    /**
     * @brief 获取当前情感状态
     * @return EmotionState 当前情感
     */
    EmotionState currentEmotion() const;

    /**
     * @brief 获取愉悦度
     * @return double 0-1
     */
    double pleasure() const { return m_state.pleasure; }

    /**
     * @brief 获取好奇度
     * @return double 0-1
     */
    double curiosity() const { return m_state.curiosity; }

    /**
     * @brief 获取疲倦度
     * @return double 0-1
     */
    double tiredness() const { return m_state.tiredness; }

Q_SIGNALS:
    /**
     * @brief 情感变化信号
     * @param state 新的情感状态
     */
    void emotionChanged(const EmotionState& state);

private:
    /**
     * @brief 裁剪值到 0-1 范围
     */
    static double clamp(double value);

    /**
     * @brief 发布情感变化事件
     */
    void publishEmotion();

    EventBus& m_eventBus;
    EmotionState m_state;
};

} // namespace core
} // namespace petapp

Q_DECLARE_METATYPE(petapp::core::EmotionState)

#endif // PETAPP_CORE_EMOTIONMODULE_H