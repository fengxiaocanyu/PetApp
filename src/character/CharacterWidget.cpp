// ============================================================
// CharacterWidget.cpp - 角色渲染部件实现（序列帧模式）
// ============================================================

#include "PlatformDefines.h"
#include "character/CharacterWidget.h"
#include "core/EventBus.h"
#include "core/EmotionModule.h"
#include "config/CharacterConfig.h"

#include <QLabel>
#include <QTimer>
#include <QDir>
#include <QApplication>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <QJsonObject>

namespace petapp {
namespace character {

CharacterWidget::CharacterWidget(core::EventBus& eventBus, QWidget* parent)
    : QWidget(parent)
    , m_eventBus(eventBus)
    , m_label(new QLabel(this))
    , m_currentFrameIndex(0)
    , m_idleTimer(new QTimer(this))
    , m_actionTimer(new QTimer(this))
    , m_currentState(CharacterState::Idle)
    , m_previousState(CharacterState::Idle)
    , m_isTalking(false)
    , m_actionFrame(0)
    , m_actionTotalFrames(0)
{
    // QLabel 设置
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet("background: transparent;");
    m_label->setGeometry(0, 0, width(), height());

    // 待机帧动画定时器（每 200ms 切换一帧）
    connect(m_idleTimer, &QTimer::timeout, this, &CharacterWidget::animateIdle);
    m_idleTimer->setInterval(200);

    // 动作定时器
    connect(m_actionTimer, &QTimer::timeout, this, [this]() {
        m_actionFrame++;
        if (m_actionFrame >= m_actionTotalFrames) {
            m_actionTimer->stop();
            setState(CharacterState::Idle);
            return;
        }
        switch (m_currentState) {
        case CharacterState::Waving:
            animateShake();
            break;
        case CharacterState::Jumping:
            animateJump();
            break;
        default:
            break;
        }
    });
    m_actionTimer->setInterval(30);

    // 订阅情感变化事件
    m_eventBus.on("emotion_changed", [this](const QVariant& variant) {
        auto state = variant.value<core::EmotionState>();
        setEmotion(state);
    });
}

CharacterWidget::~CharacterWidget()
{
    m_idleTimer->stop();
    m_actionTimer->stop();
}

void CharacterWidget::initialize(const QString& skinPath)
{
    bool loaded = false;
    if (!skinPath.isEmpty()) {
        loaded = loadFrames(skinPath);
    }

    if (!loaded || m_frames.isEmpty()) {
        // 无贴图时显示占位文本
        m_label->setText("(Pet)");
        m_label->setStyleSheet("background: transparent; color: #666; font-size: 24px;");
    } else {
        // 显示第一帧
        showFrame(0);
        // 启动待机动画
        m_idleTimer->start();
    }
}

bool CharacterWidget::loadFrames(const QString& skinPath)
{
    QDir skinDir(skinPath);
    if (!skinDir.exists()) return false;

    // 查找所有 frame_*.png 文件
    QStringList filters;
    filters << "frame_*.png";
    QFileInfoList files = skinDir.entryInfoList(filters, QDir::Files, QDir::Name);

    if (files.isEmpty()) return false;

    m_frameFiles.clear();
    m_frames.clear();

    for (const auto& fi : files) {
        QPixmap pix(fi.absoluteFilePath());
        if (!pix.isNull()) {
            m_frameFiles.append(fi.absoluteFilePath());
            m_frames.append(pix);
        }
    }

    return !m_frames.isEmpty();
}

void CharacterWidget::showFrame(int index)
{
    if (index < 0 || index >= m_frames.size()) return;
    m_currentFrameIndex = index;
    QPixmap pix = m_frames[index];
    // 根据缩放比例调整图片大小
    int targetHeight = static_cast<int>(pix.height() * m_scale);
    if (targetHeight < 50) targetHeight = 50;  // 最小 50px
    if (targetHeight > 1000) targetHeight = 1000;  // 最大 1000px
    pix = pix.scaledToHeight(targetHeight, Qt::SmoothTransformation);
    m_label->setPixmap(pix);
    // 调整窗口大小以适应图片
    resize(pix.size());
    m_label->resize(pix.size());
}

void CharacterWidget::applyConfig(const config::CharacterParams& params)
{
    // 应用缩放配置
    m_scale = params.scale;
    m_offsetX = params.offsetX;
    m_offsetY = params.offsetY;
    m_idleTimer->setInterval(params.animationInterval);

    // 重新渲染当前帧
    if (m_currentFrameIndex >= 0 && m_currentFrameIndex < m_frames.size()) {
        showFrame(m_currentFrameIndex);
    }
}

void CharacterWidget::animateIdle()
{
    if (m_frames.isEmpty()) return;
    // 循环播放帧
    int nextFrame = (m_currentFrameIndex + 1) % m_frames.size();
    showFrame(nextFrame);
}

void CharacterWidget::setState(CharacterState state)
{
    if (m_currentState == state) return;

    m_previousState = m_currentState;
    m_currentState = state;

    switch (state) {
    case CharacterState::Idle:
        // 恢复待机动画
        m_idleTimer->start();
        break;

    case CharacterState::Happy:
        // 高兴：继续待机动画（后续可替换为高兴专用帧）
        m_idleTimer->start();
        break;

    case CharacterState::Sad:
        m_idleTimer->start();
        break;

    case CharacterState::Tired:
        m_idleTimer->start();
        break;

    case CharacterState::Talking:
        m_idleTimer->start();
        break;

    case CharacterState::Waving:
        // 晃动动画
        m_idleTimer->stop();
        m_actionFrame = 0;
        m_actionTotalFrames = 20;
        m_originalPos = parentWidget()->mapToGlobal(QPoint(0, 0));
        m_actionTimer->start();
        break;

    case CharacterState::Jumping:
        // 跳跃动画
        m_idleTimer->stop();
        m_actionFrame = 0;
        m_actionTotalFrames = 30;
        m_originalPos = parentWidget()->mapToGlobal(QPoint(0, 0));
        m_actionTimer->start();
        break;

    default:
        break;
    }

    Q_EMIT stateChanged(state);
}

void CharacterWidget::playAction(const QString& action)
{
    if (action == "shake" || action == "wave") {
        setState(CharacterState::Waving);
    } else if (action == "jump") {
        setState(CharacterState::Jumping);
    } else if (action == "happy") {
        setState(CharacterState::Happy);
    } else if (action == "sad") {
        setState(CharacterState::Sad);
    } else if (action == "tired") {
        setState(CharacterState::Tired);
    }
}

void CharacterWidget::setEmotion(const core::EmotionState& emotion)
{
    if (emotion.tiredness > 0.7) {
        setState(CharacterState::Tired);
    } else if (emotion.pleasure > 0.7) {
        setState(CharacterState::Happy);
    } else if (emotion.curiosity > 0.7) {
        setState(CharacterState::Curious);
    } else if (emotion.pleasure < 0.3) {
        setState(CharacterState::Sad);
    } else {
        setState(CharacterState::Idle);
    }
}

void CharacterWidget::setTalking(bool isTalking)
{
    m_isTalking = isTalking;
    if (isTalking) {
        setState(CharacterState::Talking);
    } else {
        setState(CharacterState::Idle);
    }
}

QRectF CharacterWidget::characterBoundingRect() const
{
    return QRectF(QPointF(0, 0), size());
}

void CharacterWidget::setClickCallback(std::function<void(const QPointF&)> callback)
{
    m_clickCallback = callback;
}

void CharacterWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_clickCallback) {
            m_clickCallback(event->position());
        }
        Q_EMIT clicked(event->position());
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void CharacterWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_label->setGeometry(0, 0, width(), height());
}

void CharacterWidget::animateShake()
{
    // 左右晃动窗口
    QWidget* parent = parentWidget();
    if (!parent) return;

    qreal offset = qSin(m_actionFrame * 0.5) * 5.0;
    parent->move(m_originalPos.x() + static_cast<int>(offset), m_originalPos.y());
}

void CharacterWidget::animateJump()
{
    // 上下弹跳窗口
    QWidget* parent = parentWidget();
    if (!parent) return;

    qreal jumpProgress = static_cast<qreal>(m_actionFrame) / m_actionTotalFrames;
    qreal jumpHeight = qSin(jumpProgress * M_PI * 2) * 15.0;
    if (jumpHeight < 0) jumpHeight = 0;

    parent->move(m_originalPos.x(), m_originalPos.y() - static_cast<int>(jumpHeight));
}

} // namespace character
} // namespace petapp