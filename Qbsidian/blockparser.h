#ifndef BLOCKPARSER_H
#define BLOCKPARSER_H

#pragma once
#include <QString>
#include <QVector>

enum class BlockType
{
    Paragraph,          // 普通文本段落，无特殊标记的行累积而成
    Heading1,           // 行首 "# "
    Heading2,           // 行首 "## "
    Heading3,           // 行首 "### "
    Heading4,           // 行首 "#### "
    CodeBlockStart,     // 行首 "```"，进入代码块
    CodeBlockLine,      // 代码块内原样保留的行，仅 HTML 转义
    CodeBlockEnd,       // 行首 "```"，退出代码块
    UnorderedListItem,  // 行首 "- " / "* " / "+ "，indentLevel 为前导空格数
    OrderedListItem,    // 行首 "数字. "，indentLevel 同上
    HorizontalRule,      // 单独一行 "---" / "***" / "___"
    Blockquote          //引用块 ">"
};

struct LogicalBlock
{
    BlockType type;
    QString content;        // 去掉行首标记后的正文
    int indentLevel = 0;    // 前导空格数
    int listNumber = 0;     // OrderedListItem 记录序号
};

class BlockParser
{
public:
    static QVector<LogicalBlock> parseBlocks(const QString &text);
};

#endif // BLOCKPARSER_H
