// ============================================================
// BubbleWidget.cpp - 气泡对话框部件实现
// ============================================================

#include "PlatformDefines.h"
#include "ui/BubbleWidget.h"

#include <QLabel>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

namespace petapp {
namespace ui {

BubbleWidget::BubbleWidget(QWidget* parent)
    : QWidget(parent)
    , m_label(nullptr)
    , m_autoHideTimer(new QTimer(this))
{
    // 设置窗口属性
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);

    // 设置固定大小
    resize(260, 80);

    // 创建标签
    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true);
    m_label->setStyleSheet(
        "QLabel {"
        "  color: #333;"
        "  font-size: 13px;"
        "  padding: 10px;"
        "}"
    );

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_label);

    // 自动隐藏定时器
    m_autoHideTimer->setSingleShot(true);
    connect(m_autoHideTimer, &QTimer::timeout, this, &BubbleWidget::onAutoHide);
}

BubbleWidget::~BubbleWidget() = default;

void BubbleWidget::showMessage(const QString& text, int durationMs)
{
    m_currentText = text;
    m_label->setText(text);

    // 根据文本长度调整宽度
    int textWidth = m_label->fontMetrics().horizontalAdvance(text);
    int w = qMin(qMax(textWidth + 30, 100), 400);
    resize(w, 80);

    // 定位到父窗口底部中间
    if (parentWidget()) {
        int x = (parentWidget()->width() - width()) / 2;
        int y = parentWidget()->height() - height() - 10;
        move(x, y);
    }

    show();
    raise();

    // 启动自动隐藏
    m_autoHideTimer->start(durationMs);
}

void BubbleWidget::hideMessage()
{
    m_autoHideTimer->stop();
    hide();
}

void BubbleWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制半透明圆角背景
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);

    QColor bgColor(255, 255, 255, 220);  // 半透明白色
    painter.fillPath(path, bgColor);

    // 绘制边框
    QPen pen(QColor(200, 200, 200, 180), 1);
    painter.setPen(pen);
    painter.drawPath(path);
}

void BubbleWidget::onAutoHide()
{
    hide();
}

} // namespace ui
} // namespace petapp