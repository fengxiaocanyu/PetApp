#ifndef PETAPP_UI_SETTINGSDIALOG_H
#define PETAPP_UI_SETTINGSDIALOG_H

// ============================================================
// SettingsDialog.h - 设置对话框
// 包含网络开关、性能模式等选项
// ============================================================

#include <QDialog>
#include <QString>

class QCheckBox;
class QSlider;
class QComboBox;
class QPushButton;
class QLabel;

namespace petapp { namespace core {
    class EventBus;
    class CapabilityManager;
}}
namespace petapp { namespace config {
    class CharacterConfig;
}}

namespace petapp {
namespace ui {

/**
 * @brief 设置对话框
 *
 * 提供网络开关、性能模式、心跳间隔等设置选项。
 * 设置变更通过 EventBus 发布事件通知其他模块。
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param capabilityManager 能力管理器引用
     * @param parent 父窗口
     */
    explicit SettingsDialog(core::EventBus& eventBus,
                            core::CapabilityManager& capabilityManager,
                            config::CharacterConfig& characterConfig,
                            QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SettingsDialog() override;

private Q_SLOTS:
    /**
     * @brief 保存设置
     */
    void onSave();

    /**
     * @brief 取消设置
     */
    void onCancel();

    /**
     * @brief 网络开关变化
     */
    void onNetworkToggled(bool checked);

private:
    /**
     * @brief 初始化 UI
     */
    void setupUI();

    /**
     * @brief 加载当前设置
     */
    void loadSettings();

    core::EventBus& m_eventBus;
    core::CapabilityManager& m_capabilityManager;
    config::CharacterConfig& m_characterConfig;

    // UI 组件
    QCheckBox* m_networkCheckBox;
    QCheckBox* m_performanceModeCheckBox;
    QSlider* m_heartbeatIntervalSlider;
    QComboBox* m_skinComboBox;
    QSlider* m_scaleSlider;
    QLabel* m_scaleLabel;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
};

} // namespace ui
} // namespace petapp

#endif // PETAPP_UI_SETTINGSDIALOG_H