#ifndef REVIEWMANAGER_H
#define REVIEWMANAGER_H

#include <QMap>
#include <QList>
#include <QString>
#include <QDateTime>
#include "reviewentity.h"
#include "reviewstrategy.h"

struct ManualReviewSchedule {
    QString id;
    QString notePath;
    QString noteTitle;
    QDateTime reviewDateTime;
};

enum class ReviewPlanItemSource {
    Strategy = 0,
    ManualSchedule = 1
};

struct ReviewPlanItem {
    QString id;
    QString noteId;
    QString noteTitle;
    QDateTime reviewTime;
    ReviewPlanItemSource source = ReviewPlanItemSource::Strategy;
    int userPriority = 0;
    int reviewCount = 0;
};

class ReviewManager {
private:
    QMap<QString, ReviewEntity> m_noteEntities;
    QMap<QString, IReviewStrategy*> m_strategies;
    QList<ManualReviewSchedule> m_manualSchedules;
    int m_globalDailyLimit = 50;
    QString m_vaultPath;
    QString m_dataFilePath;
public:
    ReviewManager();
    ~ReviewManager();

    void setVaultPath(const QString &vaultPath);

    // ---------------- 系统初始化与持久化 ----------------
    void loadData();  // 从本地 .json 文件读取实体和自定义策略
    void saveData();  // 将更新后的数据写回本地磁盘

    // ---------------- 策略管理接口 ----------------
    void registerStrategy(IReviewStrategy* strategy);
    QString createCustomStrategy(const QString& name, const QList<int>& intervals, const QList<int>& restDays);
    void deleteCustomStrategy(const QString& strategyId);
    QList<QString> getAllStrategyNames() const;

    // ---------------- 笔记状态交互接口 ----------------
    // 新增笔记进入复习流
    void addNoteToReview(const QString& noteId, const QString& title, const QString& strategyId, const QString& anchor = "");
    bool hasReviewRecord(const QString& noteId) const;

    // 修改笔记绑定的策略
    void changeNoteStrategy(const QString& noteId, const QString& newStrategyId);

    // 设置优先级（用户干预优先级）
    void setNotePriority(const QString& noteId, int priority);

    // 设为已掌握（软删除）
    void markNoteAsMastered(const QString& noteId);

    // ---------------- 核心排期与复习执行接口 ----------------

    /**
     * @brief 生成今日复习计划表 (供 UI 调用)
     * @param maxDailyLimit 每日最大复习量（超过此限制的顺延）
     * @return 按“用户优先级降序 -> 逾期时间降序”排好序的待办列表
     */
    QList<ReviewEntity> generateDailyPlan(int maxDailyLimit = 50);
    QList<ReviewPlanItem> reviewPlanBetween(const QDate& start, const QDate& end) const;
    void addManualReviewSchedule(const QString& notePath, const QString& noteTitle, const QDateTime& reviewDateTime);
    void removeManualSchedule(const QString& scheduleId);
    void moveManualSchedule(const QString& scheduleId, const QDateTime& reviewDateTime);
    void removeManualSchedulesForNote(const QString& notePath);
    void removeStrategyReviewRecord(const QString& noteId);
    void excludeStrategyReviewDate(const QString& noteId, const QDate& date);
    void skipStrategyReview(const QString& noteId);

    /**
     * @brief 用户在 UI 点了“认识/不认识”后调用此接口更新状态
     * @param noteId 笔记ID
     * @param isRemembered 是否记得
     */
    void processReviewFeedback(const QString& noteId, bool isRemembered);
    // 满足需求：全局设置每天复习量
    void setGlobalDailyLimit(int limit) { m_globalDailyLimit = limit; }

    // 满足需求：对某一门笔记单独设置复习计划（指定哪天复习）
    // 调用这个函数后，该笔记会无视原有策略，被强行安排在你指定的日期
    void setSpecificReviewDate(const QString& noteId, const QDateTime& specificDate);
    // 在 ReviewManager.h 的 public 下追加：
    void removeNoteRecord(const QString& noteId);
    void renameNoteRecord(const QString& oldNoteId, const QString& newNoteId);
    void renameNoteRecordsPrefix(const QString& oldPrefix, const QString& newPrefix);
};

#endif // REVIEWMANAGER_H
