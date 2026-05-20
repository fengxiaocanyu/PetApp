#ifndef PETAPP_UI_BUBBLEWIDGET_H
#define PETAPP_UI_BUBBLEWIDGET_H

// ============================================================
// BubbleWidget.h - 气泡对话框部件
// 半透明圆角气泡，自动消失
// ============================================================

#include <QWidget>
#include <QString>

class QLabel;
class QTimer;

namespace petapp {
namespace ui {

/**
 * @brief 气泡对话框 - 显示消息气泡
 *
 * 半透明圆角气泡，默认 3 秒自动消失。
 * 用于显示桌宠的对话、提示信息等。
 */
class BubbleWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit BubbleWidget(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~BubbleWidget() override;

    /**
     * @brief 显示消息
     * @param text 消息文本
     * @param durationMs 显示时长（毫秒），默认 3000
     */
    void showMessage(const QString& text, int durationMs = 3000);

    /**
     * @brief 隐藏消息
     */
    void hideMessage();

protected:
    /**
     * @brief 绘制事件 - 绘制圆角背景
     */
    void paintEvent(QPaintEvent* event) override;

private Q_SLOTS:
    /**
     * @brief 自动隐藏定时器触发
     */
    void onAutoHide();

private:
    QLabel* m_label;
    QTimer* m_autoHideTimer;
    QString m_currentText;
};

} // namespace ui
} // namespace petapp

#endif // PETAPP_UI_BUBBLEWIDGET_H