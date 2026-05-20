// ============================================================
// ChatDialog.cpp - 对话聊天窗口实现
// ============================================================

#include "PlatformDefines.h"
#include "ui/ChatDialog.h"
#include "core/SkillManager.h"
#include "core/EventBus.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QJsonObject>
#include <QScrollBar>
#include <QDateTime>

namespace petapp {
namespace ui {

ChatDialog::ChatDialog(core::SkillManager& skillManager,
                       core::EventBus& eventBus,
                       QWidget* parent)
    : QDialog(parent)
    , m_skillManager(skillManager)
    , m_eventBus(eventBus)
    , m_chatHistory(nullptr)
    , m_inputField(nullptr)
    , m_sendButton(nullptr)
    , m_currentRequestId(0)
{
    setWindowTitle("💬 与桌宠对话");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(400, 500);
    resize(450, 550);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // 标题
    auto* titleLabel = new QLabel("💬 与桌宠聊天", this);
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #333;"
        "padding-bottom: 8px; border-bottom: 2px solid #4a9eff;"
    );
    mainLayout->addWidget(titleLabel);

    // 聊天记录
    m_chatHistory = new QTextEdit(this);
    m_chatHistory->setReadOnly(true);
    m_chatHistory->setStyleSheet(
        "QTextEdit {"
        "  background-color: #f5f5f5;"
        "  border: 1px solid #ddd;"
        "  border-radius: 8px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "}"
    );
    m_chatHistory->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainLayout->addWidget(m_chatHistory, 1);

    // 输入区域
    auto* inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(8);

    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText("输入你想说的话...");
    m_inputField->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "  background-color: white;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #4a9eff;"
        "}"
    );
    inputLayout->addWidget(m_inputField, 1);

    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setFixedSize(70, 36);
    m_sendButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #4a9eff;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3a8eef;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #2a7edf;"
        "}"
    );
    inputLayout->addWidget(m_sendButton);

    mainLayout->addLayout(inputLayout);

    // 连接信号
    connect(m_sendButton, &QPushButton::clicked, this, &ChatDialog::onSendMessage);
    connect(m_inputField, &QLineEdit::returnPressed, this, &ChatDialog::onSendMessage);

    // 添加欢迎消息
    appendMessage("桌宠", "你好！我是你的桌宠小伙伴，有什么想聊的吗？😊", false);
}

ChatDialog::~ChatDialog() = default;

void ChatDialog::onSendMessage()
{
    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) return;

    // 显示用户消息
    appendMessage("你", text, true);
    m_inputField->clear();

    // 调用对话技能
    QJsonObject params;
    params["input"] = text;

    m_currentRequestId++;
    m_skillManager.runSkill("com.petapp.chat", params,
        [this](const QJsonObject& result) {
            onSkillResult(result);
        });
}

void ChatDialog::onSkillResult(const QJsonObject& result)
{
    QString output = result.value("result").toString();
    if (output.isEmpty()) {
        output = "嗯...我不太明白你在说什么 🤔";
    }

    appendMessage("桌宠", output, false);
}

void ChatDialog::appendMessage(const QString& sender, const QString& message, bool isUser)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString color = isUser ? "#4a9eff" : "#333";
    QString bgColor = isUser ? "#e8f4ff" : "#ffffff";
    QString align = isUser ? "right" : "left";

    QString html = QString(
        "<div style='text-align: %1; margin: 8px 0;'>"
        "  <div style='display: inline-block; background-color: %2;"
        "    border-radius: 8px; padding: 8px 12px; max-width: 70%%;"
        "    text-align: left;'>"
        "    <div style='font-size: 11px; color: #999; margin-bottom: 4px;'>%3  %4</div>"
        "    <div style='font-size: 13px; color: %5;'>%6</div>"
        "  </div>"
        "</div>"
    ).arg(align, bgColor, sender, timestamp, color, message.toHtmlEscaped());

    m_chatHistory->append(html);

    // 滚动到底部
    QScrollBar* scrollBar = m_chatHistory->verticalScrollBar();
    if (scrollBar) {
        scrollBar->setValue(scrollBar->maximum());
    }
}

} // namespace ui
} // namespace petapp