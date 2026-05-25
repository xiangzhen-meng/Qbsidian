#include "ReviewStrategy.h"
#include <QDate>

// ==========================================
// 具体策略 1：经典艾宾浩斯策略实现
// ==========================================

QDateTime EbbinghausStrategy::calculateNextTime(const QDateTime& lastReview, int currentCount, bool isRemembered) {
    // 保护机制：如果 lastReview 无效，则以当前时间为基准
    QDateTime baseTime = lastReview.isValid() ? lastReview : QDateTime::currentDateTime();

    // 如果用户点了“不认识/忘记”：
    // 强制将下一次复习时间定在明天，重新开始记忆周期
    if (!isRemembered) {
        return baseTime.addDays(1);
    }

    // 艾宾浩斯标准间隔天数（简化版：1天, 2天, 4天, 7天, 15天, 30天）
    // 注意：如果是0次，说明是新学习的，下次复习是1天后
    int daysToAdd = 1;
    switch (currentCount) {
    case 0: daysToAdd = 1; break;
    case 1: daysToAdd = 2; break;
    case 2: daysToAdd = 4; break;
    case 3: daysToAdd = 7; break;
    case 4: daysToAdd = 15; break;
    default: daysToAdd = 30; break; // 大于4次后，统一定为30天复习一次
    }

    return baseTime.addDays(daysToAdd);
}


// ==========================================
// 具体策略 2：自由安排策略实现 (含休息日规避)
// ==========================================

CustomStrategy::CustomStrategy(const QString& id, const QString& name, const QList<int>& intervals, const QList<int>& restDays)
    : m_id(id), m_name(name), m_intervals(intervals), m_restDaysOfWeek(restDays) {
}

QDateTime CustomStrategy::calculateNextTime(const QDateTime& lastReview, int currentCount, bool isRemembered) {
    QDateTime baseTime = lastReview.isValid() ? lastReview : QDateTime::currentDateTime();

    // 1. 基础天数计算
    int daysToAdd = 1;
    if (!isRemembered) {
        daysToAdd = 1; // 忘记了，默认明天复习
    } else {
        // 如果自定义列表为空（防呆设计），默认加 1 天
        if (m_intervals.isEmpty()) {
            daysToAdd = 1;
        } else {
            // 根据复习次数去 intervals 数组里取间隔天数
            // 如果复习次数超过了数组长度，就一直使用数组的最后一位（比如最高间隔30天）
            int index = qMin(currentCount, (int)m_intervals.size() - 1);
            daysToAdd = m_intervals[index];
        }
    }

    // 2. 初步算出下次复习时间
    QDateTime nextTime = baseTime.addDays(daysToAdd);

    // 3. 核心逻辑：休息日规避 (解决盲点1)
    // QDate::dayOfWeek() 返回 1-7，代表周一到周日
    if (!m_restDaysOfWeek.isEmpty()) {
        int safetyCounter = 0; // 防止用户把 1-7 全设为休息日导致死循环

        while (m_restDaysOfWeek.contains(nextTime.date().dayOfWeek()) && safetyCounter < 7) {
            nextTime = nextTime.addDays(1); // 自动推迟到下一天
            safetyCounter++;
        }
    }

    return nextTime;
}
