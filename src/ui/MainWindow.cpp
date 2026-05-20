// ============================================================
// MainWindow.cpp - 主窗口实现（v2.0 改造版）
// 序列帧模式：直接使用 QLabel 显示完整角色图片
// ============================================================

#include "PlatformDefines.h"
#include "ui/MainWindow.h"
#include "ui/BubbleWidget.h"
#include "ui/SettingsDialog.h"
#include "ui/SkillMarketDialog.h"
#include "ui/SkillPopupPanel.h"
#include "ui/TrayManager.h"
#include "ui/ChatDialog.h"
#include "core/EventBus.h"
#include "core/MemoryModule.h"
#include "core/HeartbeatModule.h"
#include "core/EmotionModule.h"
#include "core/CapabilityManager.h"
#include "core/SkillManager.h"
#include "voice/VoiceManager.h"
#include "character/CharacterWidget.h"
#include "config/CharacterConfig.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QJsonObject>
#include <QApplication>
#include <QScreen>
#include <QFileDialog>
#include <QInputDialog>
#include <QRandomGenerator>

namespace petapp {
namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_eventBus(nullptr)
    , m_memoryModule(nullptr)
    , m_heartbeatModule(nullptr)
    , m_emotionModule(nullptr)
    , m_capabilityManager(nullptr)
    , m_skillManager(nullptr)
    , m_voiceManager(nullptr)
    , m_characterWidget(nullptr)
    , m_bubbleWidget(nullptr)
    , m_skillPanel(nullptr)
    , m_trayManager(nullptr)
    , m_chatDialog(nullptr)
{
    setupModules();
    setupUI();
    connectEvents();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupModules()
{
    // 获取 EventBus 单例
    m_eventBus = &core::EventBus::instance();

    // 创建核心模块
    m_memoryModule = new core::MemoryModule(this);
    m_heartbeatModule = new core::HeartbeatModule(*m_eventBus, 10000, this);
    m_emotionModule = new core::EmotionModule(*m_eventBus, this);
    m_capabilityManager = new core::CapabilityManager(*m_eventBus, this);
    m_skillManager = new core::SkillManager(*m_eventBus, this);

    // 创建表现层模块
    m_voiceManager = new voice::VoiceManager(*m_eventBus, this);

    // 创建角色配置模块
    m_characterConfig = new config::CharacterConfig(*m_eventBus, *m_memoryModule, this);

    // 初始化
    m_memoryModule->load();
    m_capabilityManager->initialize();
    m_skillManager->initialize();
    m_voiceManager->initialize();
    m_characterConfig->initialize();
}

void MainWindow::setupUI()
{
    // ============================================================
    // 窗口属性 - 透明无边框，工具窗口（不在任务栏显示）
    // ============================================================
    setWindowTitle("PetApp 桌宠");
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    // 初始窗口大小（加载贴图后会根据图片大小自动调整）
    resize(400, 600);

    // ============================================================
    // 角色部件（序列帧模式）
    // ============================================================
    m_characterWidget = new character::CharacterWidget(*m_eventBus, this);
    m_characterWidget->setGeometry(0, 0, 400, 600);

    // 尝试加载皮肤贴图（优先从 config/skins/default/ 加载）
    QString skinPath = QApplication::applicationDirPath() + "/config/skins/default";
    m_characterWidget->initialize(skinPath);

    // 根据角色大小调整窗口
    QSize charSize = m_characterWidget->size();
    resize(charSize);
    m_characterWidget->setGeometry(0, 0, charSize.width(), charSize.height());

    // ============================================================
    // 气泡部件（覆盖在角色上方）
    // ============================================================
    m_bubbleWidget = new BubbleWidget(this);
    m_bubbleWidget->hide();

    // ============================================================
    // 系统托盘
    // ============================================================
    m_trayManager = new TrayManager(this, this);
    m_trayManager->initialize("PetApp 桌宠");

    // 添加技能到托盘菜单
    auto skills = m_skillManager->availableSkills();
    for (auto it = skills.begin(); it != skills.end(); ++it) {
        m_trayManager->addSkillAction(it.key(), it.value().name);
    }

    // 连接托盘信号
    connect(m_trayManager, &TrayManager::showWindow, this, [this]() {
        show();
        raise();
        activateWindow();
    });
    connect(m_trayManager, &TrayManager::hideWindow, this, [this]() {
        hide();
    });
    connect(m_trayManager, &TrayManager::quitApp, this, [this]() {
        QApplication::quit();
    });
    connect(m_trayManager, &TrayManager::openSettings, this, [this]() {
        SettingsDialog dialog(*m_eventBus, *m_capabilityManager, *m_characterConfig, this);
        dialog.exec();
    });
    connect(m_trayManager, &TrayManager::executeSkill, this, [this](const QString& skillId) {
        executeSkill(skillId);
    });

    // ============================================================
    // 对话窗口（延迟创建）
    // ============================================================
    m_chatDialog = nullptr;

}

void MainWindow::connectEvents()
{
    // 订阅心跳事件
    m_eventBus->on("heartbeat", [this](const QVariant&) {
        onHeartbeat();
    });

    // 订阅情感变化事件
    m_eventBus->on("emotion_changed", [this](const QVariant& variant) {
        onEmotionChanged(variant);
    });

    // 订阅在线状态变化事件
    m_eventBus->on("online_status_changed", [this](const QVariant& variant) {
        onOnlineStatusChanged(variant);
    });

    // 订阅气泡显示事件（来自 ActionManager）
    m_eventBus->on("show_bubble", [this](const QVariant& variant) {
        QJsonObject data = variant.toJsonObject();
        showBubble(data.value("text").toString(),
                   data.value("duration").toInt(3000));
    });

    // 订阅角色配置变更事件
    m_eventBus->on("character_config_changed", [this](const QVariant& variant) {
        if (m_characterWidget && m_characterConfig) {
            m_characterWidget->applyConfig(m_characterConfig->params());
        }
    });

    // 启动心跳
    m_heartbeatModule->start();
}

void MainWindow::onHeartbeat()
{
    // 心跳触发情感衰减
    if (m_emotionModule) {
        m_emotionModule->decay(0.02);
    }
}

void MainWindow::onEmotionChanged(const QVariant& variant)
{
    // 情感变化由 CharacterWidget 自动处理（已订阅 emotion_changed）
}

void MainWindow::onOnlineStatusChanged(const QVariant& variant)
{
    bool online = variant.toBool();
    QString text = online ? "🌐 已连接网络" : "📴 离线模式";
    showBubble(text);
}

void MainWindow::showBubble(const QString& text, int durationMs)
{
    if (m_bubbleWidget) {
        // 气泡显示在角色上方
        QPoint bubblePos(
            (width() - m_bubbleWidget->width()) / 2,
            -m_bubbleWidget->height() - 10
        );
        m_bubbleWidget->move(bubblePos);
        m_bubbleWidget->showMessage(text, durationMs);
    }
}

void MainWindow::onCharacterClicked(const QPointF& scenePos)
{
    // 点击角色：随机触发一种反应（不弹出气泡和技能列表）
    int r = QRandomGenerator::global()->bounded(4);
    switch (r) {
    case 0:
        m_characterWidget->playAction("shake");
        break;
    case 1:
        m_characterWidget->playAction("wave");
        if (m_voiceManager && m_voiceManager->isAvailable()) {
            m_voiceManager->speak("你好呀");
        }
        break;
    case 2:
        m_characterWidget->playAction("jump");
        break;
    case 3:
        m_characterWidget->playAction("happy");
        break;
    }
}

void MainWindow::executeSkill(const QString& skillId)
{
    bool networkAvailable = m_capabilityManager->isNetworkAvailable();

    // 检查技能是否可用
    if (!m_skillManager->canRunSkill(skillId, networkAvailable)) {
        showBubble("⚠️ 该技能需要网络连接，当前处于离线模式", 3000);
        return;
    }

    if (skillId == "com.petapp.chat") {
        // 对话技能：打开独立聊天窗口
        if (!m_chatDialog) {
            m_chatDialog = new ChatDialog(*m_skillManager, *m_eventBus, nullptr);
        }
        m_chatDialog->show();
        m_chatDialog->raise();
        m_chatDialog->activateWindow();
        return;
    }

    if (skillId == "com.petapp.ocr") {
        // OCR 技能：弹出文件选择器
        QString filePath = QFileDialog::getOpenFileName(
            this, "选择图片进行文字识别",
            QString(), "图片文件 (*.png *.jpg *.bmp *.jpeg)"
        );
        if (filePath.isEmpty()) return;

        QJsonObject params;
        params["input"] = filePath;

        showBubble("🔍 正在识别图片中的文字...", 5000);
        m_skillManager->runSkill(skillId, params,
            [this](const QJsonObject& result) {
                QString output = result.value("result").toString("识别失败");
                showBubble("📝 " + output, 5000);
            });
        return;
    }

    if (skillId == "com.petapp.translate") {
        // 翻译技能：弹出输入框
        bool ok;
        QString text = QInputDialog::getText(
            this, "翻译", "请输入要翻译的英文文本：",
            QLineEdit::Normal, "", &ok
        );
        if (!ok || text.isEmpty()) return;

        QJsonObject params;
        params["input"] = text;
        QJsonObject config;
        config["src_lang"] = "en";
        config["tgt_lang"] = "zh";
        params["config"] = config;

        showBubble("🌐 正在翻译...", 3000);
        m_skillManager->runSkill(skillId, params,
            [this](const QJsonObject& result) {
                QString output = result.value("result").toString("翻译失败");
                showBubble("🌐 " + output, 5000);
            });
        return;
    }

    if (skillId == "com.petapp.echo") {
        // 回显技能：简单测试
        QJsonObject params;
        params["input"] = "Hello from PetApp!";

        m_skillManager->runSkill(skillId, params,
            [this](const QJsonObject& result) {
                QString output = result.value("result").toString("(无结果)");
                showBubble("🔊 " + output, 3000);
            });
        return;
    }

    // 未知技能
    showBubble("❓ 未知技能: " + skillId, 3000);
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // 记录拖动起始位置
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging = true;
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 关闭窗口时最小化到托盘而不是退出
    if (m_trayManager && m_trayManager->isAvailable()) {
        hide();
        m_trayManager->showNotification("PetApp 桌宠",
            "桌宠已最小化到系统托盘，双击图标恢复显示。");
        event->ignore();  // 阻止真正关闭
    } else {
        // 没有托盘时正常退出
        event->accept();
        QApplication::quit();
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        // 最小化时隐藏到托盘
        if (isMinimized() && m_trayManager && m_trayManager->isAvailable()) {
            hide();
            event->ignore();
            return;
        }
    }
    QMainWindow::changeEvent(event);
}

} // namespace ui
} // namespace petapp