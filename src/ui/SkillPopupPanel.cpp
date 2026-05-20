// ============================================================
// SkillPopupPanel.cpp - 技能弹出面板实现
// ============================================================

#include "PlatformDefines.h"
#include "ui/SkillPopupPanel.h"
#include "core/SkillManager.h"
#include "core/CapabilityManager.h"
#include "core/EventBus.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScreen>

namespace petapp {
namespace ui {

SkillPopupPanel::SkillPopupPanel(core::SkillManager& skillManager,
                                 core::CapabilityManager& capabilityManager,
                                 core::EventBus& eventBus,
                                 QWidget* parent)
    : QFrame(parent)
    , m_skillManager(skillManager)
    , m_capabilityManager(capabilityManager)
    , m_eventBus(eventBus)
    , m_layout(nullptr)
    , m_titleLabel(nullptr)
{
    // 窗口属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose, false);

    // 固定宽度
    setFixedWidth(200);

    // 阴影效果
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 3);
    setGraphicsEffect(shadow);

    // 布局
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->setSpacing(6);

    // 标题
    m_titleLabel = new QLabel("🎮 技能列表", this);
    m_titleLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #333;"
        "padding-bottom: 6px; border-bottom: 1px solid #eee;"
    );
    m_layout->addWidget(m_titleLabel);

    // 初始化技能图标映射
    m_skillIcons["com.petapp.echo"] = "🔊";
    m_skillIcons["com.petapp.translate"] = "🌐";
    m_skillIcons["com.petapp.ocr"] = "🔍";
    m_skillIcons["com.petapp.chat"] = "💬";

    // 初始刷新
    refreshSkills();

    // 弹性空间
    m_layout->addStretch();

    // 设置样式
    setStyleSheet(
        "SkillPopupPanel {"
        "  background-color: white;"
        "  border-radius: 10px;"
        "  border: 1px solid #e0e0e0;"
        "}"
    );
}

SkillPopupPanel::~SkillPopupPanel() = default;

void SkillPopupPanel::showAt(const QPoint& pos)
{
    // 确保面板在屏幕内
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenRect = screen->availableGeometry();
        int x = pos.x();
        int y = pos.y();

        // 如果超出右边界，向左显示
        if (x + width() > screenRect.right()) {
            x = pos.x() - width() - 10;
        }
        // 如果超出下边界，向上显示
        if (y + height() > screenRect.bottom()) {
            y = screenRect.bottom() - height();
        }
        // 确保不超出左边界和上边界
        x = qMax(screenRect.left() + 5, x);
        y = qMax(screenRect.top() + 5, y);

        move(x, y);
    } else {
        move(pos);
    }

    // 刷新技能列表
    refreshSkills();

    show();
    raise();
}

void SkillPopupPanel::dismiss()
{
    hide();
}

void SkillPopupPanel::setSkillClickCallback(std::function<void(const QString&)> callback)
{
    m_skillCallback = callback;
}

void SkillPopupPanel::refreshSkills()
{
    // 清除旧按钮
    for (auto* btn : m_skillButtons) {
        m_layout->removeWidget(btn);
        btn->deleteLater();
    }
    m_skillButtons.clear();

    // 获取可用技能
    auto skills = m_skillManager.availableSkills();
    bool networkAvailable = m_capabilityManager.isNetworkAvailable();

    for (auto it = skills.begin(); it != skills.end(); ++it) {
        const QString& skillId = it.key();
        const auto& info = it.value();

        // 检查技能是否可用
        bool canRun = m_skillManager.canRunSkill(skillId, networkAvailable);

        // 获取图标
        QString icon = m_skillIcons.value(skillId, "🔧");

        // 创建按钮
        QPushButton* btn = createSkillButton(skillId, info.name, icon);
        btn->setEnabled(canRun);

        if (!canRun) {
            btn->setToolTip("⚠️ 该技能需要网络连接，当前处于离线模式");
        }

        m_layout->addWidget(btn);
        m_skillButtons.append(btn);
    }

    // 如果没有技能
    if (skills.isEmpty()) {
        QLabel* emptyLabel = new QLabel("暂无可用技能", this);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #999; font-size: 12px; padding: 10px;");
        m_layout->addWidget(emptyLabel);
        m_skillButtons.append(nullptr);  // 占位
    }

    // 调整面板高度
    adjustSize();
}

QPushButton* SkillPopupPanel::createSkillButton(const QString& skillId,
                                                 const QString& displayName,
                                                 const QString& icon)
{
    auto* btn = new QPushButton(QString("%1  %2").arg(icon, displayName), this);
    btn->setFixedHeight(40);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background-color: #f8f8f8;"
        "  border: 1px solid #e8e8e8;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  text-align: left;"
        "  font-size: 13px;"
        "  color: #333;"
        "}"
        "QPushButton:hover {"
        "  background-color: #e8f4ff;"
        "  border-color: #4a9eff;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #d0ebff;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #f0f0f0;"
        "  color: #bbb;"
        "  border-color: #e0e0e0;"
        "}"
    );

    connect(btn, &QPushButton::clicked, this, [this, skillId]() {
        if (m_skillCallback) {
            m_skillCallback(skillId);
        }
        dismiss();
    });

    return btn;
}

void SkillPopupPanel::mousePressEvent(QMouseEvent* event)
{
    // 点击面板外部时关闭
    QPoint clickPos = mapToGlobal(event->pos());
    if (!rect().contains(event->pos())) {
        dismiss();
    }
    QFrame::mousePressEvent(event);
}

void SkillPopupPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 白色背景 + 圆角
    painter.setBrush(Qt::white);
    painter.setPen(QPen(QColor(224, 224, 224), 1));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);

    QFrame::paintEvent(event);
}

} // namespace ui
} // namespace petapp