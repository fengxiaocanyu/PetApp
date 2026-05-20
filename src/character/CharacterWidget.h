#ifndef PETAPP_CHARACTER_CHARACTERWIDGET_H
#define PETAPP_CHARACTER_CHARACTERWIDGET_H

// ============================================================
// CharacterWidget.h - 角色渲染部件（序列帧模式）
// 加载 config/skins/default/frame_*.png 序列帧
// 支持帧动画、表情切换、点击检测
// ============================================================

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QPoint>

class QLabel;
class QTimer;

namespace petapp { namespace core {
    class EventBus;
    struct EmotionState;
}}
namespace petapp { namespace config {
    struct CharacterParams;
}}

namespace petapp {
namespace character {

/**
 * @brief 角色状态枚举
 */
enum class CharacterState {
    Idle,       // 待机
    Happy,      // 高兴
    Sad,        // 难过
    Tired,      // 疲倦
    Curious,    // 好奇
    Talking,    // 说话
    Waving,     // 挥手
    Jumping,    // 跳跃
    Blushing    // 脸红
};

/**
 * @brief 角色渲染部件（序列帧模式）
 *
 * 加载 config/skins/default/ 下的 frame_*.png 序列帧，
 * 通过 QLabel 显示，支持帧动画和点击检测。
 */
class CharacterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterWidget(core::EventBus& eventBus, QWidget* parent = nullptr);
    ~CharacterWidget() override;

    /**
     * @brief 初始化角色
     * @param skinPath 皮肤贴图目录路径
     */
    void initialize(const QString& skinPath = "");

    /**
     * @brief 设置角色状态
     */
    void setState(CharacterState state);

    /**
     * @brief 获取当前状态
     */
    CharacterState currentState() const { return m_currentState; }

    /**
     * @brief 播放动作
     */
    void playAction(const QString& action);

    /**
     * @brief 设置情感状态
     */
    void setEmotion(const core::EmotionState& emotion);

    /**
     * @brief 说话动画
     */
    void setTalking(bool isTalking);

    /**
     * @brief 获取角色整体边界
     */
    QRectF characterBoundingRect() const;

    /**
     * @brief 获取显示用的 QLabel
     */
    QLabel* displayLabel() const { return m_label; }

    /**
     * @brief 设置点击回调
     */
    void setClickCallback(std::function<void(const QPointF&)> callback);

    /**
     * @brief 应用角色配置（缩放、偏移等）
     */
    void applyConfig(const config::CharacterParams& params);

signals:
    void stateChanged(CharacterState newState);
    void clicked(const QPointF& scenePos);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief 从目录加载序列帧
     */
    bool loadFrames(const QString& skinPath);

    /**
     * @brief 切换到指定帧
     */
    void showFrame(int index);

    /**
     * @brief 待机动画（帧循环）
     */
    void animateIdle();

    /**
     * @brief 晃动动画
     */
    void animateShake();

    /**
     * @brief 跳跃动画
     */
    void animateJump();

    core::EventBus& m_eventBus;
    QLabel* m_label;

    // 序列帧
    QStringList m_frameFiles;   // 帧文件路径列表
    QList<QPixmap> m_frames;    // 缓存的帧图片
    int m_currentFrameIndex;    // 当前帧索引

    // 动画定时器
    QTimer* m_idleTimer;        // 待机帧循环
    QTimer* m_actionTimer;      // 动作动画

    // 状态
    CharacterState m_currentState;
    CharacterState m_previousState;
    bool m_isTalking;

    // 动作参数
    int m_actionFrame;
    int m_actionTotalFrames;
    QPoint m_originalPos;       // 窗口原始位置（用于晃动/跳跃）

    // 点击回调
    std::function<void(const QPointF&)> m_clickCallback;

    // 角色配置参数
    double m_scale = 1.0;
    int m_offsetX = 0;
    int m_offsetY = 0;
};

} // namespace character
} // namespace petapp

#endif // PETAPP_CHARACTER_CHARACTERWIDGET_H