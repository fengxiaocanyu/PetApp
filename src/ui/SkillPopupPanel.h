#ifndef PETAPP_UI_SKILLPOPUPPANEL_H
#define PETAPP_UI_SKILLPOPUPPANEL_H

// ============================================================
// SkillPopupPanel.h - 技能弹出面板
// 点击角色后弹出的白色技能列表，显示可用技能
// ============================================================

#include <QFrame>
#include <QString>
#include <QMap>
#include <functional>

class QVBoxLayout;
class QPushButton;
class QLabel;

namespace petapp { namespace core {
    class SkillManager;
    class CapabilityManager;
    class EventBus;
}}

namespace petapp {
namespace ui {

/**
 * @brief 技能弹出面板
 *
 * 白色圆角面板，显示可用技能列表。
 * 点击角色任意部位时弹出，点击面板外或选择技能后关闭。
 */
class SkillPopupPanel : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param skillManager 技能管理器引用
     * @param capabilityManager 能力管理器引用
     * @param eventBus 事件总线引用
     * @param parent 父部件
     */
    explicit SkillPopupPanel(core::SkillManager& skillManager,
                             core::CapabilityManager& capabilityManager,
                             core::EventBus& eventBus,
                             QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SkillPopupPanel() override;

    /**
     * @brief 显示面板在指定位置
     * @param pos 屏幕坐标位置
     */
    void showAt(const QPoint& pos);

    /**
     * @brief 隐藏面板
     */
    void dismiss();

    /**
     * @brief 设置技能点击回调
     * @param callback 回调函数，参数为技能 ID
     */
    void setSkillClickCallback(std::function<void(const QString& skillId)> callback);

    /**
     * @brief 刷新技能列表
     */
    void refreshSkills();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    /**
     * @brief 创建技能按钮
     * @param skillId 技能 ID
     * @param displayName 显示名称
     * @param icon 图标文字
     */
    QPushButton* createSkillButton(const QString& skillId, const QString& displayName,
                                   const QString& icon);

    core::SkillManager& m_skillManager;
    core::CapabilityManager& m_capabilityManager;
    core::EventBus& m_eventBus;

    QVBoxLayout* m_layout;
    QLabel* m_titleLabel;
    QList<QPushButton*> m_skillButtons;

    std::function<void(const QString&)> m_skillCallback;

    // 技能图标映射
    QMap<QString, QString> m_skillIcons;
};

} // namespace ui
} // namespace petapp

#endif // PETAPP_UI_SKILLPOPUPPANEL_H