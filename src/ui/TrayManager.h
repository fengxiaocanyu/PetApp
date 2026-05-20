#ifndef PETAPP_UI_TRAYMANAGER_H
#define PETAPP_UI_TRAYMANAGER_H

// ============================================================
// TrayManager.h - 系统托盘管理器
// 支持最小化到系统托盘，单击弹出技能菜单
// ============================================================

#include <QObject>
#include <QString>
#include <QSystemTrayIcon>
#include <QMap>

class QMenu;
class QAction;
class QWidget;

namespace petapp {
namespace ui {

/**
 * @brief 系统托盘管理器
 *
 * 管理系统托盘图标、右键菜单。
 * 单击托盘图标弹出技能列表和退出选项。
 */
class TrayManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parentWidget 关联的主窗口
     * @param parent 父对象
     */
    explicit TrayManager(QWidget* parentWidget, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~TrayManager() override;

    /**
     * @brief 初始化托盘
     * @param toolTip 托盘图标提示文字
     */
    void initialize(const QString& toolTip = "PetApp 桌宠");

    /**
     * @brief 设置托盘图标
     * @param iconPath 图标文件路径
     */
    void setIcon(const QString& iconPath);

    /**
     * @brief 显示托盘通知
     * @param title 通知标题
     * @param message 通知内容
     * @param durationMs 显示时长（毫秒）
     */
    void showNotification(const QString& title, const QString& message, int durationMs = 3000);

    /**
     * @brief 检查托盘是否可用
     */
    bool isAvailable() const;

    /**
     * @brief 添加技能到托盘菜单
     * @param skillId 技能 ID
     * @param skillName 技能显示名称
     */
    void addSkillAction(const QString& skillId, const QString& skillName);

    /**
     * @brief 清空技能菜单
     */
    void clearSkillActions();

signals:
    /**
     * @brief 显示主窗口信号
     */
    void showWindow();

    /**
     * @brief 隐藏主窗口信号
     */
    void hideWindow();

    /**
     * @brief 退出应用信号
     */
    void quitApp();

    /**
     * @brief 打开设置信号
     */
    void openSettings();

    /**
     * @brief 执行技能信号
     * @param skillId 技能 ID
     */
    void executeSkill(const QString& skillId);

private slots:
    /**
     * @brief 托盘图标激活事件
     * @param reason 激活原因
     */
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QWidget* m_parentWidget;
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QMenu* m_skillsMenu;

    QAction* m_showAction;
    QAction* m_hideAction;
    QAction* m_settingsAction;
    QAction* m_quitAction;

    QMap<QString, QAction*> m_skillActions;
};

} // namespace ui
} // namespace petapp

#endif // PETAPP_UI_TRAYMANAGER_H