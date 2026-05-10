#include "blockparser.h"

QVector<LogicalBlock> BlockParser::parseBlocks(const QString &text)
{
    QStringList lines = text.split('\n');
    QVector<LogicalBlock> ans = {};
    bool isInCoding = false;
    QString paragraphBuffer;

    for(QString& line : lines)
    {
        // 代码块状态：原样保留，仅判断结束符
        if(isInCoding)
        {
            if(line.trimmed() == "```")
            {
                ans.push_back({BlockType::CodeBlockEnd, "", 0});
                isInCoding = false;
            }
            else
            {
                ans.push_back({BlockType::CodeBlockLine, line, 0});
                continue;
            }
        }

        int pos = 0;
        LogicalBlock tmp;
        tmp.indentLevel = 0;
        while(pos < line.length())
        {
            if(line[pos] == ' ') { tmp.indentLevel++; pos++; }
            else if(line[pos] == '\t') { tmp.indentLevel += 4; pos++; }
            else break;
        }
        QString remaining = line.mid(pos);

        // 空行：段落结束
        if(remaining.isEmpty())
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            continue;
        }

        // 判断是否为 ``` 开始行
        if(remaining.startsWith("```"))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::CodeBlockStart, remaining.mid(3).trimmed(), tmp.indentLevel});
            isInCoding = true;
            continue;

        }
        // [此处填写] 列表续行判断
        // 条件：当前行不是列表标记 && ans 非空 && ans.last() 是列表项 && tmp.indentLevel >= ans.last().indentLevel + 2
        // 若是：ans.last().content += " " + remaining.trimmed(); continue;

        // 标题判断（#### / ### / ## / #）
        else if(remaining.startsWith("####"))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading4,remaining.mid(4).trimmed(),tmp.indentLevel});
        }
        else if(remaining.startsWith("###"))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading3,remaining.mid(3).trimmed(),tmp.indentLevel});
        }
        else if(remaining.startsWith("##"))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading2,remaining.mid(2).trimmed(),tmp.indentLevel});
        }
        else if(remaining.startsWith("#"))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading1,remaining.mid(1).trimmed(),tmp.indentLevel});
        }
        // [此处填写] 无序列表判断（- / * / +）
        // 匹配分支内：
        //   if(!paragraphBuffer.isEmpty()) { ans.push_back({BlockType::Paragraph, paragraphBuffer, 0}); paragraphBuffer.clear(); }
        //   tmp.type = BlockType::UnorderedListItem; tmp.content = remaining.mid(2); ans.push_back(tmp); continue;

        // [此处填写] 有序列表判断（数字. ）
        // 匹配分支内：
        //   if(!paragraphBuffer.isEmpty()) { ans.push_back({BlockType::Paragraph, paragraphBuffer, 0}); paragraphBuffer.clear(); }
        //   tmp.type = BlockType::OrderedListItem; tmp.content = ...; tmp.listNumber = ...; ans.push_back(tmp); continue;

        // [此处填写] 水平分割线判断（--- / *** / ___）
        // 匹配分支内：
        //   if(!paragraphBuffer.isEmpty()) { ans.push_back({BlockType::Paragraph, paragraphBuffer, 0}); paragraphBuffer.clear(); }
        //   tmp.type = BlockType::HorizontalRule; tmp.content = ""; ans.push_back(tmp); continue;

        // 普通段落行：累积到缓冲区
        if(!paragraphBuffer.isEmpty())
            paragraphBuffer += " ";
        paragraphBuffer += remaining;
    }

    // 文件结束，flush 剩余段落
    if(!paragraphBuffer.isEmpty())
    {
        ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
    }

    return ans;
}
