#ifndef QUESTIONDATA_H
#define QUESTIONDATA_H

#include <QString>

struct ExtractedQuestion
{
    QString id;          // 唯一标识 (建议用: 文件路径_行号 生成哈希，保证同一道题ID不变)
    QString sourceFile;  // 来源文件名
    QString tag;         // 题库分类/标签 (如: 计概)
    QString question;    // 问题 (Markdown格式)
    QString answer;      // 答案 (Markdown格式)
};

#endif // QUESTIONDATA_H
