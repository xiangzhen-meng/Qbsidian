#include "questionextractor.h"
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
QuestionExtractor::QuestionExtractor(QObject *parent) : QObject(parent)
{
    // 初始化正则表达式
    // 语法解释:
    // #题库/(\S+)    -> 捕获组1: 匹配标签(不含空格的连续字符)
    // \s+           -> 匹配中间的空格
    // (.+?)         -> 捕获组2: 匹配问题 (非贪婪模式)
    // ::            -> 分隔符
    // (.+)          -> 捕获组3: 匹配答案 (直到行尾)
    m_regex.setPattern("#题库/(\\S+)\\s+(.+?)::(.+)");
}

QList<ExtractedQuestion> QuestionExtractor::extractAllFromDirectory(const QString &rootDirectory)
{
    QList<ExtractedQuestion> allQuestions;

    // 使用 QDirIterator 递归遍历所有 .md 文件
    QDirIterator it(rootDirectory, QStringList() << "*.md", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        allQuestions.append(extractFromFile(filePath));
    }

    return allQuestions;
}

QList<ExtractedQuestion> QuestionExtractor::extractFromFile(const QString &filePath)
{
    QList<ExtractedQuestion> fileQuestions;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fileQuestions; // 文件打开失败，返回空
    }

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif

    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        // 如果该行不包含 "::" 或 "#题库"，直接跳过，加速判断
        if (!line.contains("::") || !line.contains("#题库/")) {
            continue;
        }

        // 进行正则匹配
        QRegularExpressionMatch match = m_regex.match(line);
        if (match.hasMatch()) {
            ExtractedQuestion q;
            q.tag = match.captured(1).trimmed();       // 提取标签
            q.question = match.captured(2).trimmed();  // 提取问题
            q.answer = match.captured(3).trimmed();    // 提取答案
            q.sourceFile = filePath;
            q.id = generateUniqueId(filePath, lineNumber);

            fileQuestions.append(q);
        }
    }

    file.close();
    return fileQuestions;
}

QString QuestionExtractor::generateUniqueId(const QString &filePath, int lineNumber)
{
    // 利用 "文件路径+行号" 生成 MD5 哈希作为该题的唯一 ID
    // 这样只要题目在原笔记的原位置，复习规划器就能认出它，保留它的复习进度
    QString rawString = filePath + "_" + QString::number(lineNumber);
    QByteArray hash = QCryptographicHash::hash(rawString.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}
// 实现从预制 JSON 资源中读取题目
QList<ExtractedQuestion> QuestionExtractor::extractFromPreset(const QString &resourcePath)
{
    QList<ExtractedQuestion> presetQuestions;

    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开预制题库资源文件，请检查路径是否正确:" << resourcePath;
        return presetQuestions;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "预制题库 JSON 解析失败:" << parseError.errorString();
        return presetQuestions;
    }

    if (jsonDoc.isArray()) {
        QJsonArray jsonArray = jsonDoc.array();
        for (int i = 0; i < jsonArray.size(); ++i) {
            QJsonObject jsonObj = jsonArray[i].toObject();

            ExtractedQuestion q;
            // 读取 JSON 键值，并赋给你的结构体属性
            q.id = jsonObj["id"].toString();
            q.sourceFile = "System Preset";  // 来源标记为系统预制
            q.tag = jsonObj["subject"].toString(); // JSON里的科目(如:程设)对应 tag
            q.question = jsonObj["q"].toString();  // JSON里的问题对应 question
            q.answer = jsonObj["a"].toString();    // JSON里的答案对应 answer

            presetQuestions.append(q);
        }
    }

    qDebug() << "成功从资源中提取了" << presetQuestions.size() << "道预制题目！";
    return presetQuestions;
}
