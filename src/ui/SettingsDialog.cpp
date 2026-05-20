// ============================================================
// SettingsDialog.cpp - 设置对话框实现
// ============================================================

#include "PlatformDefines.h"
#include "ui/SettingsDialog.h"
#include "core/EventBus.h"
#include "core/CapabilityManager.h"
#include "config/CharacterConfig.h"

#include <QCheckBox>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QJsonObject>
#include <QDir>
#include <QFileInfoList>

namespace petapp {
namespace ui {

SettingsDialog::SettingsDialog(core::EventBus& eventBus,
                               core::CapabilityManager& capabilityManager,
                               config::CharacterConfig& characterConfig,
                               QWidget* parent)
    : QDialog(parent)
    , m_eventBus(eventBus)
    , m_capabilityManager(capabilityManager)
    , m_characterConfig(characterConfig)
    , m_networkCheckBox(nullptr)
    , m_performanceModeCheckBox(nullptr)
    , m_heartbeatIntervalSlider(nullptr)
    , m_skinComboBox(nullptr)
    , m_scaleSlider(nullptr)
    , m_scaleLabel(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("设置");
    setMinimumWidth(420);
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // ============================================================
    // 网络设置组
    // ============================================================
    auto* networkGroup = new QGroupBox("网络设置", this);
    auto* networkLayout = new QVBoxLayout(networkGroup);

    m_networkCheckBox = new QCheckBox("启用网络功能", networkGroup);
    connect(m_networkCheckBox, &QCheckBox::toggled,
            this, &SettingsDialog::onNetworkToggled);
    networkLayout->addWidget(m_networkCheckBox);

    auto* networkHint = new QLabel("关闭后所有需要网络的技能将被禁用", networkGroup);
    networkHint->setStyleSheet("color: #888; font-size: 11px;");
    networkLayout->addWidget(networkHint);

    mainLayout->addWidget(networkGroup);

    // ============================================================
    // 性能设置组
    // ============================================================
    auto* perfGroup = new QGroupBox("性能设置", this);
    auto* perfLayout = new QFormLayout(perfGroup);

    m_performanceModeCheckBox = new QCheckBox("性能模式（减少动画效果）", perfGroup);
    perfLayout->addRow("模式:", m_performanceModeCheckBox);

    // 心跳间隔滑块
    auto* heartbeatLayout = new QHBoxLayout();
    m_heartbeatIntervalSlider = new QSlider(Qt::Horizontal, perfGroup);
    m_heartbeatIntervalSlider->setRange(1, 30);
    m_heartbeatIntervalSlider->setTickPosition(QSlider::TicksBelow);
    m_heartbeatIntervalSlider->setTickInterval(5);
    auto* heartbeatLabel = new QLabel("10 秒", perfGroup);
    heartbeatLabel->setMinimumWidth(50);
    connect(m_heartbeatIntervalSlider, &QSlider::valueChanged, this,
        [heartbeatLabel](int value) {
            heartbeatLabel->setText(QString("%1 秒").arg(value));
        });
    heartbeatLayout->addWidget(m_heartbeatIntervalSlider);
    heartbeatLayout->addWidget(heartbeatLabel);
    perfLayout->addRow("心跳间隔:", heartbeatLayout);

    mainLayout->addWidget(perfGroup);

    // ============================================================
    // 外观设置组
    // ============================================================
    auto* appearanceGroup = new QGroupBox("外观设置", this);
    auto* appearanceLayout = new QFormLayout(appearanceGroup);

    // 皮肤选择
    m_skinComboBox = new QComboBox(appearanceGroup);
    m_skinComboBox->addItem("默认皮肤", "default");

    // 扫描 config/skins/ 目录下的皮肤
    QDir skinsDir("config/skins");
    if (skinsDir.exists()) {
        QStringList skinDirs = skinsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto& skin : skinDirs) {
            m_skinComboBox->addItem(skin, skin);
        }
    }

    appearanceLayout->addRow("皮肤:", m_skinComboBox);

    // 角色缩放滑块
    auto* scaleLayout = new QHBoxLayout();
    m_scaleSlider = new QSlider(Qt::Horizontal, appearanceGroup);
    m_scaleSlider->setRange(10, 300);  // 10% ~ 300%
    m_scaleSlider->setTickPosition(QSlider::TicksBelow);
    m_scaleSlider->setTickInterval(25);
    m_scaleLabel = new QLabel("100%", appearanceGroup);
    m_scaleLabel->setMinimumWidth(50);
    connect(m_scaleSlider, &QSlider::valueChanged, this,
        [this](int value) {
            m_scaleLabel->setText(QString("%1%").arg(value));
        });
    scaleLayout->addWidget(m_scaleSlider);
    scaleLayout->addWidget(m_scaleLabel);
    appearanceLayout->addRow("角色缩放:", scaleLayout);

    mainLayout->addWidget(appearanceGroup);

    // ============================================================
    // 系统模块状态组
    // ============================================================
    auto* moduleGroup = new QGroupBox("系统模块状态", this);
    auto* moduleLayout = new QVBoxLayout(moduleGroup);

    auto* ttsStatus = new QLabel("语音模块 (TTS): 预留", moduleGroup);
    ttsStatus->setStyleSheet("color: #888; font-size: 11px;");
    moduleLayout->addWidget(ttsStatus);

    auto* skinStatus = new QLabel("外观模块 (Appearance): 已集成到 CharacterWidget", moduleGroup);
    skinStatus->setStyleSheet("color: #888; font-size: 11px;");
    moduleLayout->addWidget(skinStatus);

    auto* actionStatus = new QLabel("动作模块 (Action): 已集成到 CharacterWidget", moduleGroup);
    actionStatus->setStyleSheet("color: #888; font-size: 11px;");
    moduleLayout->addWidget(actionStatus);

    mainLayout->addWidget(moduleGroup);

    // ============================================================
    // 按钮区域
    // ============================================================
    mainLayout->addStretch();

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_saveButton = new QPushButton("保存", this);
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::onSave);
    buttonLayout->addWidget(m_saveButton);

    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancel);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::loadSettings()
{
    // 加载网络状态
    bool online = m_capabilityManager.isOnline();
    m_networkCheckBox->setChecked(online);

    // 加载性能模式
    m_performanceModeCheckBox->setChecked(false);

    // 加载心跳间隔
    m_heartbeatIntervalSlider->setValue(10);

    // 加载角色缩放
    double scale = m_characterConfig.scale();
    int scalePercent = static_cast<int>(scale * 100);
    m_scaleSlider->setValue(scalePercent);
    m_scaleLabel->setText(QString("%1%").arg(scalePercent));
}

void SettingsDialog::onNetworkToggled(bool checked)
{
    if (checked) {
        m_capabilityManager.setOnline(true);
    } else {
        m_capabilityManager.setOffline();
    }
}

void SettingsDialog::onSave()
{
    // 保存角色缩放
    double scale = m_scaleSlider->value() / 100.0;
    m_characterConfig.setScale(scale);

    // 发布设置变更事件
    QJsonObject settings;
    settings["network_enabled"] = m_networkCheckBox->isChecked();
    settings["performance_mode"] = m_performanceModeCheckBox->isChecked();
    settings["heartbeat_interval"] = m_heartbeatIntervalSlider->value();
    settings["skin"] = m_skinComboBox->currentData().toString();
    settings["character_scale"] = scale;

    m_eventBus.emitEvent("settings_changed", QVariant(settings));

    accept();
}

void SettingsDialog::onCancel()
{
    reject();
}

} // namespace ui
} // namespace petapp