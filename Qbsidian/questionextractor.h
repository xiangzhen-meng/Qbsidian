#ifndef QUESTIONEXTRACTOR_H
#define QUESTIONEXTRACTOR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QRegularExpression>
#include "QuestionData.h"

class QuestionExtractor : public QObject
{
    QObject *parent = nullptr;

public:
    explicit QuestionExtractor(QObject *parent = nullptr);

    // 核心接口：传入知识库的根目录，返回提取出的所有题目
    QList<ExtractedQuestion> extractAllFromDirectory(const QString &rootDirectory);

    // 辅助接口：仅提取单个文件
    QList<ExtractedQuestion> extractFromFile(const QString &filePath);
    // 预设题库
    QList<ExtractedQuestion> extractFromPreset(const QString &resourcePath = ":/presets/preset_cs.json");

private:
    // 正则表达式对象，提升为成员变量避免重复编译
    QRegularExpression m_regex;

    // 生成唯一ID的辅助函数
    QString generateUniqueId(const QString &filePath, int lineNumber);
};

#endif // QUESTIONEXTRACTOR_H
