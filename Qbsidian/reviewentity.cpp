#include "reviewentity.h"
#include <QJsonArray>
#include <QJsonValue>


ReviewEntity::ReviewEntity()
{

    noteId = "";
    noteTitle = "";
    jumpAnchor = "";
    strategyId = "";


    status = NoteStatus::Learning; // 默认状态为“学习中”
    userPriority = 0;              // 默认普通优先级
    reviewCount = 0;               // 还没复习过，次数为0

    // 时间初始化
    createTime = QDateTime::currentDateTime(); // 记录创建的精确时间
    // lastReviewTime 和 nextReviewTime 保持无效状态 (isValid() == false)，
    // 等待 ReviewManager 赋予初始策略计算出的时间
}


QJsonObject ReviewEntity::toJson() const
{
    QJsonObject json;

    json["noteId"] = noteId;
    json["noteTitle"] = noteTitle;
    json["jumpAnchor"] = jumpAnchor;
    json["strategyId"] = strategyId;


    json["status"] = static_cast<int>(status);
    json["userPriority"] = userPriority;
    json["reviewCount"] = reviewCount;

    // 时间处理
    json["createTime"] = createTime.isValid() ? createTime.toString(Qt::ISODate) : "";
    json["lastReviewTime"] = lastReviewTime.isValid() ? lastReviewTime.toString(Qt::ISODate) : "";
    json["nextReviewTime"] = nextReviewTime.isValid() ? nextReviewTime.toString(Qt::ISODate) : "";

    QJsonArray excludedArray;
    for (const QDate &date : excludedReviewDates) {
        if (date.isValid())
            excludedArray.append(date.toString(Qt::ISODate));
    }
    json["excludedReviewDates"] = excludedArray;

    return json;
}



void ReviewEntity::fromJson(const QJsonObject& json) {
    // 读取基础信息
    noteId = json["noteId"].toString();
    noteTitle = json["noteTitle"].toString();
    jumpAnchor = json["jumpAnchor"].toString();
    strategyId = json["strategyId"].toString();

    // 读取状态与数值
    status = static_cast<NoteStatus>(json["status"].toInt(0)); // 默认0(Learning)
    userPriority = json["userPriority"].toInt(0);
    reviewCount = json["reviewCount"].toInt(0);

    // 时间处理
    QString createStr = json["createTime"].toString();
    if (!createStr.isEmpty()) {
        createTime = QDateTime::fromString(createStr, Qt::ISODate);
    }

    QString lastStr = json["lastReviewTime"].toString();
    if (!lastStr.isEmpty()) {
        lastReviewTime = QDateTime::fromString(lastStr, Qt::ISODate);
    }

    QString nextStr = json["nextReviewTime"].toString();
    if (!nextStr.isEmpty()) {
        nextReviewTime = QDateTime::fromString(nextStr, Qt::ISODate);
    }

    excludedReviewDates.clear();
    QJsonArray excludedArray = json["excludedReviewDates"].toArray();
    for (const QJsonValue &value : excludedArray) {
        QDate date = QDate::fromString(value.toString(), Qt::ISODate);
        if (date.isValid() && !excludedReviewDates.contains(date))
            excludedReviewDates.append(date);
    }
}
