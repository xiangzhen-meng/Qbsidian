#ifndef REVIEWSTRATEGY_H
#define REVIEWSTRATEGY_H

#include <QDateTime>
#include <QString>
#include <QList>

// ==========================================
// 抽象策略基类
// ==========================================
class IReviewStrategy {
public:
    virtual ~IReviewStrategy() = default;

    // 获取策略唯一标识（如 "ebbinghaus", "custom_math"）
    virtual QString getStrategyId() const = 0;

    // 获取策略展示名称
    virtual QString getStrategyName() const = 0;

    /**
     * 核心算法：计算下次复习时间
     * @param lastReview 上次复习的具体时间
     * @param currentCount 已经成功复习的次数
     * @param isRemembered 本次复习是否记得（如果不记得，策略内部可决定是否将时间缩短或重置次数）
     * @return 计算出的下次复习时间
     */
    virtual QDateTime calculateNextTime(const QDateTime& lastReview, int currentCount, bool isRemembered) = 0;
};

// ==========================================
// 具体策略 1：经典艾宾浩斯
// ==========================================
class EbbinghausStrategy : public IReviewStrategy {
public:
    QString getStrategyId() const override { return "standard_ebbinghaus"; }
    QString getStrategyName() const override { return "经典艾宾浩斯策略"; }
    QDateTime calculateNextTime(const QDateTime& lastReview, int currentCount, bool isRemembered) override;
};

// ==========================================
// 具体策略 2：自由安排策略
// ==========================================
class CustomStrategy : public IReviewStrategy {
private:
    QString m_id;
    QString m_name;
    QList<int> m_intervals;       // 用户自定义的间隔天数，例如：{1, 2, 5, 10, 30}
    QList<int> m_restDaysOfWeek;  // 设定的休息日（如 6, 7 代表周六周日）

public:
    CustomStrategy(const QString& id, const QString& name, const QList<int>& intervals, const QList<int>& restDays);

    // 在 ReviewStrategy.h 的 CustomStrategy 类中补充：
    QList<int> getIntervals() const { return m_intervals; }
    QList<int> getRestDays() const { return m_restDaysOfWeek; }
    QString getStrategyId() const override { return m_id; }
    QString getStrategyName() const override { return m_name; }

    // 内部实现会自动计算：如果算出来的那天是 m_restDaysOfWeek，则自动顺延
    QDateTime calculateNextTime(const QDateTime& lastReview, int currentCount, bool isRemembered) override;
};

#endif // REVIEWSTRATEGY_H
