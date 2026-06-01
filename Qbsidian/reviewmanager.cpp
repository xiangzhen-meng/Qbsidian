#include "ReviewManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QFileInfo>
#include <algorithm>

// ==========================================
// 构造与析构函数
// ==========================================
ReviewManager::ReviewManager() {
    m_globalDailyLimit = 50; // 默认每日复习上限

    // 1. 注册系统内置的标准艾宾浩斯策略
    registerStrategy(new EbbinghausStrategy());

    // 2. 加载本地数据（会自动加载自定义策略和笔记数据）
    loadData();
}

ReviewManager::~ReviewManager() {
    // 安全释放所有动态分配的策略内存，防止内存泄漏
    qDeleteAll(m_strategies);
    m_strategies.clear();
}

// ==========================================
// 系统初始化与持久化（JSON 读写逻辑）
// ==========================================

void ReviewManager::loadData() {
    QFile file("review_data.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "未找到 review_data.json，系统将初始化空数据。";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject root = doc.object();

    // 1. 读取全局每日复习上限
    if (root.contains("globalDailyLimit")) {
        m_globalDailyLimit = root["globalDailyLimit"].toInt(50);
    }

    // 2. 读取并注册所有的自定义策略
    if (root.contains("customStrategies")) {
        QJsonArray stratArray = root["customStrategies"].toArray();
        for (auto val : stratArray) {
            QJsonObject stratObj = val.toObject();
            QString id = stratObj["id"].toString();
            QString name = stratObj["name"].toString();

            // 转换复习周期
            QList<int> intervals;
            QJsonArray interArr = stratObj["intervals"].toArray();
            for (auto i : interArr) intervals.append(i.toInt());

            // 转换休息日
            QList<int> restDays;
            QJsonArray restArr = stratObj["restDays"].toArray();
            for (auto r : restArr) restDays.append(r.toInt());

            // 注册到内存
            registerStrategy(new CustomStrategy(id, name, intervals, restDays));
        }
    }

    // 3. 读取所有笔记的复习状态
    if (root.contains("notes")) {
        QJsonArray notesArray = root["notes"].toArray();
        m_noteEntities.clear();
        for (auto val : notesArray) {
            ReviewEntity entity;
            entity.fromJson(val.toObject());
            m_noteEntities.insert(entity.noteId, entity);
        }
    }

    m_manualSchedules.clear();
    if (root.contains("manualSchedules")) {
        QJsonArray scheduleArray = root["manualSchedules"].toArray();
        for (const QJsonValue &val : scheduleArray) {
            QJsonObject obj = val.toObject();
            ManualReviewSchedule schedule;
            schedule.id = obj["id"].toString();
            schedule.notePath = obj["notePath"].toString();
            schedule.noteTitle = obj["noteTitle"].toString();
            schedule.reviewDateTime = QDateTime::fromString(obj["reviewDateTime"].toString(), Qt::ISODate);
            if (!schedule.notePath.isEmpty() && schedule.reviewDateTime.isValid())
                m_manualSchedules.append(schedule);
        }
    }
}

void ReviewManager::saveData() {
    QJsonObject root;
    root["globalDailyLimit"] = m_globalDailyLimit;

    // 1. 保存所有自定义策略（不保存内置的 ebbinghaus 策略）
    QJsonArray stratArray;
    for (auto strategy : m_strategies.values()) {
        if (strategy->getStrategyId() != "standard_ebbinghaus") {
            // 利用 dynamic_cast 安全转换
            auto customStrat = dynamic_cast<CustomStrategy*>(strategy);
            if (customStrat) {
                QJsonObject stratObj;
                stratObj["id"] = customStrat->getStrategyId();
                stratObj["name"] = customStrat->getStrategyName();

                QJsonArray interArr;
                for (int i : customStrat->getIntervals()) interArr.append(i);
                stratObj["intervals"] = interArr;

                QJsonArray restArr;
                for (int r : customStrat->getRestDays()) restArr.append(r);
                stratObj["restDays"] = restArr;

                stratArray.append(stratObj);
            }
        }
    }
    root["customStrategies"] = stratArray;

    // 2. 保存所有笔记实体状态
    QJsonArray notesArray;
    for (const auto& entity : m_noteEntities.values()) {
        notesArray.append(entity.toJson());
    }
    root["notes"] = notesArray;

    QJsonArray scheduleArray;
    for (const ManualReviewSchedule &schedule : m_manualSchedules) {
        QJsonObject obj;
        obj["id"] = schedule.id;
        obj["notePath"] = schedule.notePath;
        obj["noteTitle"] = schedule.noteTitle;
        obj["reviewDateTime"] = schedule.reviewDateTime.toString(Qt::ISODate);
        scheduleArray.append(obj);
    }
    root["manualSchedules"] = scheduleArray;

    // 3. 写入文件
    QFile file("review_data.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented)); // 使用缩进格式保存，方便阅读
        file.close();
    } else {
        qWarning() << "无法写入本地 review_data.json 数据文件";
    }
}

// ==========================================
// 策略管理
// ==========================================

void ReviewManager::registerStrategy(IReviewStrategy* strategy) {
    if (!strategy) return;

    // 如果策略已存在，先删掉旧的，再覆盖（防止重复注册内存溢出）
    if (m_strategies.contains(strategy->getStrategyId())) {
        delete m_strategies.value(strategy->getStrategyId());
    }
    m_strategies.insert(strategy->getStrategyId(), strategy);
}

void ReviewManager::createCustomStrategy(const QString& name, const QList<int>& intervals, const QList<int>& restDays) {
    // 生成一个基于时间戳的唯一ID
    QString id = QString("custom_%1").arg(QDateTime::currentMSecsSinceEpoch());
    CustomStrategy* newStrat = new CustomStrategy(id, name, intervals, restDays);
    registerStrategy(newStrat);
    saveData();
}

void ReviewManager::deleteCustomStrategy(const QString& strategyId) {
    if (strategyId == "standard_ebbinghaus") return; // 禁止删除标准策略

    if (m_strategies.contains(strategyId)) {
        delete m_strategies.take(strategyId); // 从 Map 移除并释放内存

        // 避坑：把所有绑定了这个策略的笔记，自动降级回艾宾浩斯策略，防止崩溃
        for (auto& entity : m_noteEntities) {
            if (entity.strategyId == strategyId) {
                entity.strategyId = "standard_ebbinghaus";
            }
        }
        saveData();
    }
}

QList<QString> ReviewManager::getAllStrategyNames() const {
    QList<QString> names;
    for (auto strat : m_strategies.values()) {
        names.append(strat->getStrategyName());
    }
    return names;
}

// ==========================================
// 实体与交互管理
// ==========================================

void ReviewManager::addNoteToReview(const QString& noteId, const QString& title, const QString& strategyId, const QString& anchor) {
    // 如果笔记已经在复习队列，不重复添加
    if (m_noteEntities.contains(noteId)) return;

    ReviewEntity entity;
    entity.noteId = noteId;
    entity.noteTitle = title;
    entity.jumpAnchor = anchor;
    entity.strategyId = m_strategies.contains(strategyId) ? strategyId : "standard_ebbinghaus";

    // 核心计算：新笔记加入时，由策略算出它的第一次复习时间
    IReviewStrategy* strat = m_strategies.value(entity.strategyId);
    entity.nextReviewTime = strat->calculateNextTime(entity.createTime, 0, true);

    m_noteEntities.insert(noteId, entity);
    saveData();
}

void ReviewManager::changeNoteStrategy(const QString& noteId, const QString& newStrategyId) {
    if (!m_noteEntities.contains(noteId) || !m_strategies.contains(newStrategyId)) return;

    ReviewEntity& entity = m_noteEntities[noteId];
    entity.strategyId = newStrategyId;

    // 切换策略后，依据上次复习时间，用新策略重新计算下次复习时间
    IReviewStrategy* strat = m_strategies.value(newStrategyId);
    entity.nextReviewTime = strat->calculateNextTime(entity.lastReviewTime, entity.reviewCount, true);

    saveData();
}

void ReviewManager::setNotePriority(const QString& noteId, int priority) {
    if (m_noteEntities.contains(noteId)) {
        m_noteEntities[noteId].userPriority = priority;
        saveData();
    }
}

void ReviewManager::markNoteAsMastered(const QString& noteId) {
    if (m_noteEntities.contains(noteId)) {
        m_noteEntities[noteId].status = NoteStatus::Mastered; // 软删除状态
        saveData();
    }
}

void ReviewManager::setSpecificReviewDate(const QString& noteId, const QDateTime& specificDate) {
    if (m_noteEntities.contains(noteId)) {
        m_noteEntities[noteId].nextReviewTime = specificDate; // 强制覆写时间
        saveData();
    }
}

// ==========================================
// 核心排期与复习反馈算法（重要）
// ==========================================

QList<ReviewEntity> ReviewManager::generateDailyPlan(int maxDailyLimit) {
    // 如果未传入具体上限，使用全局变量
    int limit = (maxDailyLimit == 50) ? m_globalDailyLimit : maxDailyLimit;

    QList<ReviewEntity> dueList;
    QDateTime now = QDateTime::currentDateTime();

    // 1. 过滤到期且活跃复习中的笔记
    for (const auto& entity : m_noteEntities.values()) {
        if (entity.status == NoteStatus::Learning &&
            entity.nextReviewTime.isValid() &&
            entity.nextReviewTime <= now) {
            dueList.append(entity);
        }
    }

    // 2. 核心双条件排序（解决盲点2）
    // 排序规则：用户优先级大（更优先） > 逾期时间早（时间小代表逾期更久，更优先）
    std::sort(dueList.begin(), dueList.end(), [](const ReviewEntity& a, const ReviewEntity& b) {
        if (a.userPriority != b.userPriority) {
            return a.userPriority > b.userPriority;
        }
        return a.nextReviewTime < b.nextReviewTime;
    });

    // 3. 截断队列，保障每日复习上限（不超载）
    if (dueList.size() > limit) {
        dueList = dueList.mid(0, limit);
    }

    return dueList;
}

QList<ReviewEntity> ReviewManager::reviewPlanBetween(const QDate& start, const QDate& end) const {
    QList<ReviewEntity> plan;

    for (const auto& entity : m_noteEntities.values()) {
        if (entity.status != NoteStatus::Learning || !entity.nextReviewTime.isValid())
            continue;

        QDate reviewDate = entity.nextReviewTime.date();
        if (reviewDate >= start && reviewDate <= end)
            plan.append(entity);
    }

    for (const ManualReviewSchedule &schedule : m_manualSchedules) {
        QDate reviewDate = schedule.reviewDateTime.date();
        if (reviewDate < start || reviewDate > end)
            continue;

        ReviewEntity entity;
        entity.noteId = schedule.notePath;
        entity.noteTitle = schedule.noteTitle.isEmpty() ? QFileInfo(schedule.notePath).completeBaseName() : schedule.noteTitle;
        entity.nextReviewTime = schedule.reviewDateTime;
        entity.status = NoteStatus::Learning;
        plan.append(entity);
    }

    std::sort(plan.begin(), plan.end(), [](const ReviewEntity& a, const ReviewEntity& b) {
        if (a.nextReviewTime.date() != b.nextReviewTime.date())
            return a.nextReviewTime < b.nextReviewTime;
        if (a.userPriority != b.userPriority)
            return a.userPriority > b.userPriority;
        return a.noteTitle.compare(b.noteTitle, Qt::CaseInsensitive) < 0;
    });

    return plan;
}

void ReviewManager::addManualReviewSchedule(const QString& notePath, const QString& noteTitle, const QDateTime& reviewDateTime) {
    if (notePath.isEmpty() || !reviewDateTime.isValid())
        return;

    for (const ManualReviewSchedule &schedule : m_manualSchedules) {
        if (schedule.notePath == notePath && schedule.reviewDateTime.date() == reviewDateTime.date())
            return;
    }

    ManualReviewSchedule schedule;
    schedule.id = QString("manual_%1").arg(QDateTime::currentMSecsSinceEpoch());
    schedule.notePath = notePath;
    schedule.noteTitle = noteTitle;
    schedule.reviewDateTime = reviewDateTime;
    m_manualSchedules.append(schedule);
    saveData();
}

void ReviewManager::removeManualSchedulesForNote(const QString& notePath) {
    bool removed = false;
    for (int i = m_manualSchedules.size() - 1; i >= 0; --i) {
        if (m_manualSchedules.at(i).notePath == notePath) {
            m_manualSchedules.removeAt(i);
            removed = true;
        }
    }

    if (removed)
        saveData();
}

void ReviewManager::processReviewFeedback(const QString& noteId, bool isRemembered) {
    if (!m_noteEntities.contains(noteId)) return;

    ReviewEntity& entity = m_noteEntities[noteId];
    QDateTime now = QDateTime::currentDateTime();

    // 1. 判定是否记住
    if (isRemembered) {
        // 成功，复习次数加一，并交给策略计算下一次时间
        entity.reviewCount++;
    } else {
        // 忘记，复习次数归零（可设为0，重新开始记忆曲线）
        entity.reviewCount = 0;
    }

    entity.lastReviewTime = now;

    // 2. 调度策略，推导下一次时间
    IReviewStrategy* strat = m_strategies.value(entity.strategyId, m_strategies.value("standard_ebbinghaus"));
    entity.nextReviewTime = strat->calculateNextTime(now, entity.reviewCount, isRemembered);

    saveData(); // 状态发生改变，立即存盘
}
void ReviewManager::removeNoteRecord(const QString& noteId) {
    if (m_noteEntities.contains(noteId)) {
        m_noteEntities.remove(noteId); // 从内存 Map 中移除
        saveData();                   // 立即保存同步到磁盘
    }
    removeManualSchedulesForNote(noteId);
}
