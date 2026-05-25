#ifndef REVIEWENTITY_H
#define REVIEWENTITY_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

// 笔记当前的复习状态
enum class NoteStatus {
    Learning = 0,   // 学习中：正常参与日常复习流
    Mastered = 1,   // 已掌握：软删除，不再出现在复习计划中，但数据保留
    Suspended = 2   // 暂停：用户手动冻结（备用状态）
};

// 复习数据实体类
class ReviewEntity {
public:
    // --- 核心标识与 UI 跳转信息 ---
    QString noteId;            // 核心主键（建议用相对路径或UUID）
    QString noteTitle;         // 笔记的标题（供 UI 列表展示用）
    QString jumpAnchor;        // 跳转锚点/行号（可选，供 UI 自动滚动到特定题目使用）

    // --- 状态与策略绑定 ---
    QString strategyId;        // 绑定的复习策略ID
    NoteStatus status;         // 当前状态
    int userPriority;          // 用户设定的优先级（0为普通，正数越高越优先）

    // --- 时间与进度追踪 ---
    QDateTime createTime;      // 首次加入复习的时间
    QDateTime lastReviewTime;  // 上一次复习时间
    QDateTime nextReviewTime;  // 计算出的下次复习时间
    int reviewCount;           // 连续成功的复习次数

    // 默认构造
    ReviewEntity();

    // 序列化与反序列化（供保存到本地 JSON 使用）
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

#endif // REVIEWENTITY_H
