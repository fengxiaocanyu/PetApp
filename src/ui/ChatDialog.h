#ifndef PETAPP_UI_CHATDIALOG_H
#define PETAPP_UI_CHATDIALOG_H

// ============================================================
// ChatDialog.h - 对话聊天窗口
// 用于对话技能的独立交互窗口
// ============================================================

#include <QDialog>
#include <QString>

class QTextEdit;
class QLineEdit;
class QPushButton;

namespace petapp { namespace core {
    class SkillManager;
    class EventBus;
}}

namespace petapp {
namespace ui {

/**
 * @brief 对话聊天窗口
 *
 * 提供聊天界面，与对话技能交互。
 * 显示对话历史，支持输入和发送消息。
 */
class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param skillManager 技能管理器引用
     * @param eventBus 事件总线引用
     * @param parent 父部件
     */
    explicit ChatDialog(core::SkillManager& skillManager,
                        core::EventBus& eventBus,
                        QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ChatDialog() override;

private slots:
    /**
     * @brief 发送消息
     */
    void onSendMessage();

    /**
     * @brief 处理技能响应
     * @param result 技能返回结果
     */
    void onSkillResult(const QJsonObject& result);

private:
    /**
     * @brief 添加消息到聊天记录
     * @param sender 发送者名称
     * @param message 消息内容
     * @param isUser 是否为用户消息
     */
    void appendMessage(const QString& sender, const QString& message, bool isUser);

    core::SkillManager& m_skillManager;
    core::EventBus& m_eventBus;

    QTextEdit* m_chatHistory;
    QLineEdit* m_inputField;
    QPushButton* m_sendButton;

    int m_currentRequestId;
};

} // namespace ui
} // namespace petapp

#endif // PETAPP_UI_CHATDIALOG_H