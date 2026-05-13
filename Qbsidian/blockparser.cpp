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
                continue;
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

        //有序列表匹配
        bool isOrderedList = false;
        int orderedNum = 0;
        QString orderedContent;
        {
            int i = 0;
            while(i < remaining.length() && remaining[i].isDigit())
                i++;
            if(i > 0 && i + 1 < remaining.length() && remaining[i] == '.' && remaining[i+1] == ' ')
            {
                isOrderedList = true;
                orderedNum = remaining.left(i).toInt();
                orderedContent = remaining.mid(i + 2);
            }
        }
        bool isListMarker = remaining.startsWith("- ") || remaining.startsWith("+ ") || remaining.startsWith("* ") || isOrderedList;

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

        // 列表续行判断
        if(!isListMarker && !ans.isEmpty()&&(ans.last().type==BlockType::UnorderedListItem||ans.last().type==BlockType::OrderedListItem)
            &&tmp.indentLevel >= ans.last().indentLevel + 2)
        {
            ans.last().content += " " + remaining.trimmed();
            continue;
        }

        // 标题判断（#### / ### / ## / #）
        else if(remaining.startsWith("#### "))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading4,remaining.mid(5).trimmed(),tmp.indentLevel});
            continue;
        }
        else if(remaining.startsWith("### "))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading3,remaining.mid(4).trimmed(),tmp.indentLevel});
            continue;
        }
        else if(remaining.startsWith("## "))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading2,remaining.mid(3).trimmed(),tmp.indentLevel});
            continue;
        }
        else if(remaining.startsWith("# "))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Heading1,remaining.mid(2).trimmed(),tmp.indentLevel});
            continue;
        }

        //  无序列表判断（- / * / +）
        else if(remaining.startsWith("- ")||remaining.startsWith("* ")||remaining.startsWith("+ "))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0}); paragraphBuffer.clear();
            }
            tmp.type = BlockType::UnorderedListItem;
            tmp.content = remaining.mid(2);
            ans.push_back(tmp);
            continue;

        }

        // 有序列表判断（数字. ）
        else if(isOrderedList)
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::OrderedListItem, orderedContent.trimmed(), tmp.indentLevel, orderedNum});
            continue;
        }

        // 水平分割线判断（--- / *** / ___）
        else if(remaining.trimmed()=="---"||remaining.trimmed()=="***"||remaining.trimmed()=="___")
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            tmp.type = BlockType::HorizontalRule;
            tmp.content = "";
            ans.push_back(tmp);
            continue;
        }

        else if(remaining.startsWith("> "))
        {
            if(!paragraphBuffer.isEmpty())
            {
                ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
                paragraphBuffer.clear();
            }
            ans.push_back({BlockType::Blockquote, remaining.mid(2).trimmed(), tmp.indentLevel});
            continue;
        }

        // 普通段落行：累积到缓冲区
        if(!paragraphBuffer.isEmpty()) paragraphBuffer += " ";
        paragraphBuffer += remaining;
    }

    // 文件结束，flush 剩余段落
    if(!paragraphBuffer.isEmpty())
    {
        ans.push_back({BlockType::Paragraph, paragraphBuffer, 0});
    }

    return ans;
}
