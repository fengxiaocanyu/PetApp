#ifndef PETAPP_CONFIG_CHARACTERCONFIG_H
#define PETAPP_CONFIG_CHARACTERCONFIG_H

// ============================================================
// CharacterConfig.h - 角色参数配置模块
// 管理角色缩放、位置偏移、动画速度等参数
// 通过 EventBus 发布配置变更事件
// ============================================================

#include <QObject>
#include <QString>
#include <QVariant>

namespace petapp { namespace core {
    class EventBus;
    class MemoryModule;
}}

namespace petapp {
namespace config {

/**
 * @brief 角色参数结构体
 */
struct CharacterParams {
    double scale = 1.0;           // 缩放比例 (0.1 ~ 3.0)
    int offsetX = 0;              // 水平偏移 (像素)
    int offsetY = 0;              // 垂直偏移 (像素)
    int animationInterval = 200;  // 帧动画间隔 (毫秒)
    bool showBubble = true;       // 是否显示气泡
};

/**
 * @brief 角色参数配置模块
 *
 * 管理角色相关的所有可配置参数。
 * 参数变更时通过 EventBus 发布 "character_config_changed" 事件。
 * 参数持久化到 MemoryModule。
 */
class CharacterConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param memoryModule 存储模块引用
     * @param parent 父对象
     */
    explicit CharacterConfig(core::EventBus& eventBus,
                             core::MemoryModule& memoryModule,
                             QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~CharacterConfig() override;

    /**
     * @brief 初始化（从存储加载配置）
     */
    void initialize();

    /**
     * @brief 获取当前角色参数
     */
    CharacterParams params() const { return m_params; }

    /**
     * @brief 设置缩放比例
     * @param scale 缩放值 (0.1 ~ 3.0)
     */
    void setScale(double scale);

    /**
     * @brief 获取缩放比例
     */
    double scale() const { return m_params.scale; }

    /**
     * @brief 设置水平偏移
     */
    void setOffsetX(int offsetX);

    /**
     * @brief 获取水平偏移
     */
    int offsetX() const { return m_params.offsetX; }

    /**
     * @brief 设置垂直偏移
     */
    void setOffsetY(int offsetY);

    /**
     * @brief 获取垂直偏移
     */
    int offsetY() const { return m_params.offsetY; }

    /**
     * @brief 设置动画间隔
     */
    void setAnimationInterval(int ms);

    /**
     * @brief 获取动画间隔
     */
    int animationInterval() const { return m_params.animationInterval; }

    /**
     * @brief 设置是否显示气泡
     */
    void setShowBubble(bool show);

    /**
     * @brief 是否显示气泡
     */
    bool showBubble() const { return m_params.showBubble; }

signals:
    /**
     * @brief 配置变更信号
     */
    void configChanged(const CharacterParams& params);

private:
    /**
     * @brief 保存配置到存储
     */
    void saveConfig();

    /**
     * @brief 发布配置变更事件
     */
    void emitConfigChanged();

    core::EventBus& m_eventBus;
    core::MemoryModule& m_memoryModule;
    CharacterParams m_params;
};

} // namespace config
} // namespace petapp

#endif // PETAPP_CONFIG_CHARACTERCONFIG_H