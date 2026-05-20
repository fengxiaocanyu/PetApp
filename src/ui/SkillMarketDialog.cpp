// ============================================================
// SkillMarketDialog.cpp - 技能市场对话框实现
// ============================================================

#include "PlatformDefines.h"
#include "ui/SkillMarketDialog.h"
#include "core/EventBus.h"
#include "core/CapabilityManager.h"
#include "core/SkillManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

namespace petapp {
namespace ui {

SkillMarketDialog::SkillMarketDialog(core::EventBus& eventBus,
                                     core::CapabilityManager& capabilityManager,
                                     core::SkillManager& skillManager,
                                     QWidget* parent)
    : QDialog(parent)
    , m_eventBus(eventBus)
    , m_capabilityManager(capabilityManager)
    , m_skillManager(skillManager)
    , m_skillList(nullptr)
    , m_refreshBtn(nullptr)
    , m_downloadBtn(nullptr)
    , m_statusLabel(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
{
    setupUI();

    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &SkillMarketDialog::onNetworkReply);
}

SkillMarketDialog::~SkillMarketDialog() = default;

void SkillMarketDialog::setupUI()
{
    setWindowTitle("技能市场");
    setMinimumSize(500, 400);
    resize(550, 450);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // 标题
    auto* titleLabel = new QLabel("📦 技能市场 - 浏览和下载技能插件", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 状态标签
    m_statusLabel = new QLabel("点击「刷新列表」获取可用技能", this);
    m_statusLabel->setStyleSheet("color: #666;");
    mainLayout->addWidget(m_statusLabel);

    // 技能列表
    m_skillList = new QListWidget(this);
    m_skillList->setAlternatingRowColors(true);
    m_skillList->setStyleSheet(
        "QListWidget { border: 1px solid #ccc; border-radius: 4px; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:alternate { background-color: #f5f5f5; }"
    );
    mainLayout->addWidget(m_skillList, 1);

    // 按钮区域
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_refreshBtn = new QPushButton("🔄 刷新列表", this);
    m_refreshBtn->setMinimumHeight(36);
    connect(m_refreshBtn, &QPushButton::clicked, this, &SkillMarketDialog::onRefresh);
    buttonLayout->addWidget(m_refreshBtn);

    m_downloadBtn = new QPushButton("⬇️ 下载选中技能", this);
    m_downloadBtn->setMinimumHeight(36);
    m_downloadBtn->setEnabled(false);
    connect(m_downloadBtn, &QPushButton::clicked, this, &SkillMarketDialog::onDownload);
    buttonLayout->addWidget(m_downloadBtn);

    auto* closeBtn = new QPushButton("关闭", this);
    closeBtn->setMinimumHeight(36);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);

    // 列表选择变化时启用/禁用下载按钮
    connect(m_skillList, &QListWidget::currentRowChanged, this, [this](int row) {
        m_downloadBtn->setEnabled(row >= 0 && row < m_remoteSkills.size());
    });
}

void SkillMarketDialog::onRefresh()
{
    if (!m_capabilityManager.isNetworkAvailable()) {
        QMessageBox::warning(this, "网络不可用",
                             "当前处于离线模式，无法获取技能列表。\n"
                             "请在设置中开启网络后重试。");
        return;
    }

    m_statusLabel->setText("正在获取技能列表...");
    m_refreshBtn->setEnabled(false);

    // 请求技能市场 API（示例 URL，实际部署时替换）
    QUrl apiUrl("https://api.petapp.dev/skills/list");
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_networkManager->get(request);
}

void SkillMarketDialog::onDownload()
{
    int row = m_skillList->currentRow();
    if (row < 0 || row >= m_remoteSkills.size()) {
        return;
    }

    if (!m_capabilityManager.isNetworkAvailable()) {
        QMessageBox::warning(this, "网络不可用",
                             "当前处于离线模式，无法下载技能。\n"
                             "请在设置中开启网络后重试。");
        return;
    }

    QJsonObject skill = m_remoteSkills[row].toObject();
    QString skillId = skill.value("id").toString();
    QString skillName = skill.value("name").toString();
    QString downloadUrl = skill.value("download_url").toString();

    m_statusLabel->setText(QString("正在下载: %1...").arg(skillName));
    m_downloadBtn->setEnabled(false);

    // 下载技能包
    QUrl downloadQUrl(downloadUrl);
    QNetworkRequest request(downloadQUrl);
    m_networkManager->get(request);
}

void SkillMarketDialog::onNetworkReply(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        m_statusLabel->setText(QString("网络错误: %1").arg(reply->errorString()));
        m_refreshBtn->setEnabled(true);
        m_downloadBtn->setEnabled(true);
        return;
    }

    QByteArray responseData = reply->readAll();
    QUrl url = reply->url();

    if (url.toString().contains("/skills/list")) {
        // 解析技能列表
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            m_statusLabel->setText("解析技能列表失败");
            m_refreshBtn->setEnabled(true);
            return;
        }

        m_remoteSkills = doc.array();
        updateSkillList();
        m_statusLabel->setText(QString("找到 %1 个可用技能").arg(m_remoteSkills.size()));
        m_refreshBtn->setEnabled(true);
    } else {
        // 下载技能包 - 保存到 plugins/ 目录
        QString appDir = QCoreApplication::applicationDirPath();
        QString pluginsDir = appDir + "/plugins";

        // 从 URL 中提取文件名
        QString fileName = QFileInfo(url.path()).fileName();
        if (fileName.isEmpty()) {
            fileName = "skill_downloaded.zip";
        }

        QString savePath = pluginsDir + "/" + fileName;
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(responseData);
            file.close();
            m_statusLabel->setText(QString("下载完成: %1").arg(fileName));

            // 触发技能管理器重新扫描
            m_skillManager.scanSkills();

            QMessageBox::information(this, "下载完成",
                                     QString("技能包已下载到: %1\n"
                                             "技能管理器将自动加载新技能。")
                                     .arg(savePath));
        } else {
            m_statusLabel->setText("保存文件失败");
            QMessageBox::warning(this, "下载失败", "无法保存技能文件到 plugins/ 目录");
        }

        m_downloadBtn->setEnabled(true);
    }
}

void SkillMarketDialog::updateSkillList()
{
    m_skillList->clear();

    for (const QJsonValue& val : m_remoteSkills) {
        QJsonObject skill = val.toObject();
        QString id = skill.value("id").toString();
        QString name = skill.value("name").toString();
        QString version = skill.value("version").toString("1.0.0");
        QString description = skill.value("description").toString();
        bool requiresNetwork = skill.value("requires_network").toBool(false);

        // 检查是否已安装
        bool installed = m_skillManager.hasSkill(id);

        QString status = installed ? " [已安装]" : "";
        QString networkIcon = requiresNetwork ? " 🌐" : "";

        QString displayText = QString("%1 v%2%3%4\n  %5")
            .arg(name)
            .arg(version)
            .arg(status)
            .arg(networkIcon)
            .arg(description);

        auto* item = new QListWidgetItem(displayText, m_skillList);
        if (installed) {
            item->setForeground(QColor("#4CAF50"));
        }
    }
}

} // namespace ui
} // namespace petapp