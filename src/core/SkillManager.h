#ifndef PETAPP_CORE_SKILLMANAGER_H
#define PETAPP_CORE_SKILLMANAGER_H

// ============================================================
// SkillManager.h - 技能管理器
// 扫描 plugins/ 目录下的技能 exe，管理技能生命周期
// ============================================================

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QMap>
#include <QFileSystemWatcher>
#include <QProcess>
#include <functional>

namespace petapp { namespace core {
    class EventBus;
}}

namespace petapp {
namespace core {

/**
 * @brief 技能信息结构体
 */
struct SkillInfo {
    QString id;                 ///< 技能唯一标识
    QString name;               ///< 技能显示名称
    QString version;            ///< 版本号
    bool requiresNetwork = false;  ///< 是否需要网络
    int resourceWeight = 1;     ///< 资源消耗权重 (0-10)
    QString executable;         ///< 可执行文件名
    QString description;        ///< 描述
    QString manifestPath;       ///< manifest.json 路径
    QString exePath;            ///< exe 完整路径
};

/**
 * @brief 技能管理器 - 管理技能插件的加载、运行和热加载
 *
 * 扫描 plugins/ 目录下的 *.exe，读取同名的 manifest.json。
 * 提供 runSkill() 方法启动子进程，通过 stdin/stdout JSON 通信。
 * 使用 QFileSystemWatcher 监视 plugins/ 目录实现热加载。
 */
class SkillManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param eventBus 事件总线引用
     * @param parent 父对象
     */
    explicit SkillManager(EventBus& eventBus, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SkillManager() override;

    /**
     * @brief 初始化（扫描技能目录）
     */
    void initialize();

    /**
     * @brief 扫描 plugins/ 目录，加载所有技能
     */
    void scanSkills();

    /**
     * @brief 运行技能
     * @param skillId 技能 ID
     * @param params 参数 JSON
     * @param callback 完成回调（接收结果 JSON）
     * @return true 如果技能成功启动
     */
    bool runSkill(const QString& skillId,
                  const QJsonObject& params,
                  std::function<void(const QJsonObject&)> callback);

    /**
     * @brief 获取所有已注册的技能
     * @return QMap<QString, SkillInfo> 技能 ID 到信息的映射
     */
    QMap<QString, SkillInfo> availableSkills() const;

    /**
     * @brief 根据 ID 获取技能信息
     * @param skillId 技能 ID
     * @return SkillInfo 技能信息（如果找到）
     */
    SkillInfo skillInfo(const QString& skillId) const;

    /**
     * @brief 检查技能是否存在
     * @param skillId 技能 ID
     * @return true 如果技能存在
     */
    bool hasSkill(const QString& skillId) const;

    /**
     * @brief 检查技能是否可运行（考虑网络和资源条件）
     * @param skillId 技能 ID
     * @param isNetworkAvailable 网络是否可用
     * @return true 如果技能可运行
     */
    bool canRunSkill(const QString& skillId, bool isNetworkAvailable) const;

    /**
     * @brief 获取需要网络的技能列表
     * @return QStringList 需要网络的技能 ID 列表
     */
    QStringList skillsRequiringNetwork() const;

Q_SIGNALS:
    /**
     * @brief 技能列表更新信号
     */
    void skillsUpdated();

    /**
     * @brief 技能执行完成信号
     * @param skillId 技能 ID
     * @param result 结果 JSON
     * @param success 是否成功
     */
    void skillCompleted(const QString& skillId, const QJsonObject& result, bool success);

private Q_SLOTS:
    /**
     * @brief 技能进程完成槽函数
     */
    void onProcessFinished();

    /**
     * @brief 目录变化槽函数（热加载）
     */
    void onDirectoryChanged(const QString& path);

private:
    /**
     * @brief 从 manifest.json 加载技能信息
     * @param manifestPath manifest.json 路径
     * @return SkillInfo 解析后的技能信息
     */
    SkillInfo loadManifest(const QString& manifestPath);

    /**
     * @brief 解析技能进程的 stdout 输出
     * @param output 原始输出
     * @return QJsonObject 解析后的 JSON
     */
    QJsonObject parseProcessOutput(const QByteArray& output);

    /**
     * @brief 清理技能进程资源
     */
    void cleanupProcess();

    EventBus& m_eventBus;
    QMap<QString, SkillInfo> m_skills;
    QFileSystemWatcher* m_fileWatcher;
    QProcess* m_activeProcess;
    QString m_activeSkillId;
    std::function<void(const QJsonObject&)> m_activeCallback;
    int m_timeoutMs = 5000;
};

} // namespace core
} // namespace petapp

#endif // PETAPP_CORE_SKILLMANAGER_H