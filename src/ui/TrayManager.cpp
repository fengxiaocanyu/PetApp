// ============================================================
// TrayManager.cpp - 系统托盘管理器实现
// 单击托盘图标弹出技能菜单，双击显示/隐藏窗口
// ============================================================

#include "PlatformDefines.h"
#include "ui/TrayManager.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QApplication>
#include <QIcon>
#include <QPainter>

namespace petapp {
namespace ui {

TrayManager::TrayManager(QWidget* parentWidget, QObject* parent)
    : QObject(parent)
    , m_parentWidget(parentWidget)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_skillsMenu(nullptr)
    , m_showAction(nullptr)
    , m_hideAction(nullptr)
    , m_settingsAction(nullptr)
    , m_quitAction(nullptr)
{
}

TrayManager::~TrayManager()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

void TrayManager::initialize(const QString& toolTip)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    // 创建托盘图标
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip(toolTip);

    // 设置默认图标（使用应用图标或内置图标）
    QIcon appIcon = QApplication::windowIcon();
    if (appIcon.isNull()) {
        // 创建一个简单的默认图标
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(100, 150, 255));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 28, 28);
        painter.setBrush(Qt::white);
        painter.drawEllipse(10, 10, 6, 6);
        painter.drawEllipse(18, 10, 6, 6);
        painter.end();
        appIcon = QIcon(pixmap);
    }
    m_trayIcon->setIcon(appIcon);

    // 创建右键菜单
    m_trayMenu = new QMenu();

    // 技能子菜单
    m_skillsMenu = m_trayMenu->addMenu("🎮 技能");
    m_skillsMenu->setToolTipsVisible(true);

    m_trayMenu->addSeparator();

    m_showAction = m_trayMenu->addAction("📺 显示桌宠");
    connect(m_showAction, &QAction::triggered, this, &TrayManager::showWindow);

    m_hideAction = m_trayMenu->addAction("🙈 隐藏桌宠");
    connect(m_hideAction, &QAction::triggered, this, &TrayManager::hideWindow);

    m_trayMenu->addSeparator();

    m_settingsAction = m_trayMenu->addAction("⚙️ 设置");
    connect(m_settingsAction, &QAction::triggered, this, &TrayManager::openSettings);

    m_trayMenu->addSeparator();

    m_quitAction = m_trayMenu->addAction("🚪 退出");
    connect(m_quitAction, &QAction::triggered, this, &TrayManager::quitApp);

    m_trayIcon->setContextMenu(m_trayMenu);

    // 连接托盘图标事件
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &TrayManager::onTrayActivated);

    // 显示托盘图标
    m_trayIcon->show();
}

void TrayManager::setIcon(const QString& iconPath)
{
    if (m_trayIcon) {
        QIcon icon(iconPath);
        if (!icon.isNull()) {
            m_trayIcon->setIcon(icon);
        }
    }
}

void TrayManager::showNotification(const QString& title, const QString& message, int durationMs)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(title, message,
                                QSystemTrayIcon::Information, durationMs);
    }
}

bool TrayManager::isAvailable() const
{
    return m_trayIcon != nullptr && QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayManager::addSkillAction(const QString& skillId, const QString& skillName)
{
    if (!m_skillsMenu) return;

    // 避免重复添加
    if (m_skillActions.contains(skillId)) return;

    QAction* action = m_skillsMenu->addAction(skillName);
    m_skillActions[skillId] = action;

    // 连接信号：点击技能菜单项时发射 executeSkill 信号
    connect(action, &QAction::triggered, this, [this, skillId]() {
        Q_EMIT executeSkill(skillId);
    });
}

void TrayManager::clearSkillActions()
{
    if (!m_skillsMenu) return;

    m_skillsMenu->clear();
    m_skillActions.clear();
}

void TrayManager::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        // 双击托盘图标显示/隐藏窗口
        if (m_parentWidget->isVisible()) {
            Q_EMIT hideWindow();
        } else {
            Q_EMIT showWindow();
        }
        break;
    case QSystemTrayIcon::Trigger:
        // 单击托盘图标：弹出菜单（Qt 会自动显示 contextMenu）
        break;
    default:
        break;
    }
}

} // namespace ui
} // namespace petapp