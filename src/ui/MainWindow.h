#ifndef PETAPP_UI_MAINWINDOW_H
#define PETAPP_UI_MAINWINDOW_H

// ============================================================
// MainWindow.h - 主窗口（v2.0 改造版）
// 透明无边框窗口，显示角色部件，点击弹出技能列表
// 支持系统托盘最小化
// ============================================================

#include <QMainWindow>
#include <QString>
#include <QPoint>

class QLabel;
class QPushButton;
class QVBoxLayout;

namespace petapp { namespace core {
    class EventBus;
    class MemoryModule;
    class HeartbeatModule;
    class EmotionModule;
    struct EmotionState;
    class CapabilityManager;
    class SkillManager;
}}
namespace petapp { namespace ui {
    class BubbleWidget;
    class SettingsDialog;
    class SkillMarketDialog;
    class SkillPopupPanel;
    class TrayManager;
    class ChatDialog;
}}
namespace petapp { namespace voice {
    class VoiceManager;
}}
namespace petapp { namespace character {
    class CharacterWidget;
}}
namespace petapp { namespace config {
    class CharacterConfig;
}}

namespace petapp {
namespace ui {

/**
 * @brief 主窗口 - 桌宠主界面（v2.0）
 *
 * 透明无边框窗口，只显示角色部件。
 * 点击角色不同部位触发不同反应，弹出技能列表。
 * 支持最小化到系统托盘。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MainWindow() override;

protected:
    /**
     * @brief 鼠标按下事件（用于拖动窗口）
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief 鼠标移动事件（用于拖动窗口）
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief 关闭事件（最小化到托盘）
     */
    void closeEvent(QCloseEvent* event) override;

    /**
     * @brief 改变事件（窗口显示/隐藏）
     */
    void changeEvent(QEvent* event) override;

private:
    /**
     * @brief 初始化所有模块
     */
    void setupModules();

    /**
     * @brief 初始化 UI
     */
    void setupUI();

    /**
     * @brief 连接事件
     */
    void connectEvents();

    /**
     * @brief 心跳事件处理
     */
    void onHeartbeat();

    /**
     * @brief 情感变化事件处理
     */
    void onEmotionChanged(const QVariant& variant);

    /**
     * @brief 在线状态变化事件处理
     */
    void onOnlineStatusChanged(const QVariant& variant);

    /**
     * @brief 显示气泡消息
     */
    void showBubble(const QString& text, int durationMs = 3000);

    /**
     * @brief 处理角色点击
     * @param scenePos 点击位置
     */
    void onCharacterClicked(const QPointF& scenePos);

    /**
     * @brief 执行技能
     * @param skillId 技能 ID
     */
    void executeSkill(const QString& skillId);

    // 核心模块
    core::EventBus* m_eventBus;
    core::MemoryModule* m_memoryModule;
    core::HeartbeatModule* m_heartbeatModule;
    core::EmotionModule* m_emotionModule;
    core::CapabilityManager* m_capabilityManager;
    core::SkillManager* m_skillManager;

    // 表现层模块
    voice::VoiceManager* m_voiceManager;

    // 角色模块
    character::CharacterWidget* m_characterWidget;
    config::CharacterConfig* m_characterConfig;

    // UI 组件
    BubbleWidget* m_bubbleWidget;
    SkillPopupPanel* m_skillPanel;
    TrayManager* m_trayManager;
    ChatDialog* m_chatDialog;

    // 拖动状态
    QPoint m_dragPosition;
    bool m_dragging = false;
};

} // namespace ui
} // namespace petapp

#endif // PETAPP_UI_MAINWINDOW_H