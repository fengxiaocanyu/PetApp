#ifndef PETAPP_UI_SKILLMARKETDIALOG_H
#define PETAPP_UI_SKILLMARKETDIALOG_H

// ============================================================
// SkillMarketDialog.h - 技能市场对话框
// 联网浏览和下载技能插件
// ============================================================

#include <QDialog>
#include <QString>
#include <QJsonArray>

class QListWidget;
class QPushButton;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;

namespace petapp { namespace core {
    class EventBus;
    class CapabilityManager;
    class SkillManager;
}}

namespace petapp {
namespace ui {

/**
 * @brief 技能市场对话框
 *
 * 联网浏览可用技能，下载并安装到 plugins/ 目录。
 * 需要网络连接时可用。
 */
class SkillMarketDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param capabilityManager 能力管理器引用
     * @param skillManager 技能管理器引用
     * @param parent 父窗口
     */
    explicit SkillMarketDialog(core::EventBus& eventBus,
                               core::CapabilityManager& capabilityManager,
                               core::SkillManager& skillManager,
                               QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SkillMarketDialog() override;

private Q_SLOTS:
    /**
     * @brief 刷新技能列表
     */
    void onRefresh();

    /**
     * @brief 下载技能
     */
    void onDownload();

    /**
     * @brief 网络响应处理
     */
    void onNetworkReply(QNetworkReply* reply);

private:
    /**
     * @brief 初始化 UI
     */
    void setupUI();

    /**
     * @brief 更新技能列表显示
     */
    void updateSkillList();

    core::EventBus& m_eventBus;
    core::CapabilityManager& m_capabilityManager;
    core::SkillManager& m_skillManager;

    QListWidget* m_skillList;
    QPushButton* m_refreshBtn;
    QPushButton* m_downloadBtn;
    QLabel* m_statusLabel;
    QNetworkAccessManager* m_networkManager;

    // 从服务器获取的技能列表
    QJsonArray m_remoteSkills;
};

} // namespace ui
} // namespace petapp

#endif // PETAPP_UI_SKILLMARKETDIALOG_H