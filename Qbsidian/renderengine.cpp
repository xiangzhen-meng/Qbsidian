#include "renderengine.h"
#include "inlineparser.h"

namespace {

struct ListNode
{
    LogicalBlock block;
    QVector<ListNode> children;
};

bool isListBlock(const LogicalBlock &block)
{
    return block.type == BlockType::UnorderedListItem || block.type == BlockType::OrderedListItem;
}

QString htmlIndent(int depth)
{
    QString indent;
    for(int i = 0; i < depth * 4; ++i)
        indent += "&nbsp;";
    return indent;
}

QVector<ListNode> buildListTree(const QVector<LogicalBlock> &blocks, int &index, int end, int currentIndent)
{
    QVector<ListNode> nodes;

    while(index < end)
    {
        int indent = blocks[index].indentLevel;
        if(indent < currentIndent)
            break;

        if(indent > currentIndent)
        {
            if(!nodes.isEmpty())
                nodes.last().children = buildListTree(blocks, index, end, indent);
            else
                nodes = buildListTree(blocks, index, end, indent);
            continue;
        }

        ListNode node;
        node.block = blocks[index];
        nodes.push_back(node);
        ++index;
    }

    return nodes;
}

QString renderListTree(const QVector<ListNode> &nodes, int depth)
{
    QString html;
    for(const ListNode &node : nodes)
    {
        bool isOrdered = node.block.type == BlockType::OrderedListItem;
        QString marker = isOrdered ? QString::number(node.block.listNumber) + "." : "&bull;";
        html += QString("<p class=\"md-list-item\">%1<span class=\"md-list-marker\">%2&nbsp;</span>%3</p>")
                    .arg(htmlIndent(depth))
                    .arg(marker)
                    .arg(InlineParser::process(node.block.content));

        if(!node.children.isEmpty())
            html += renderListTree(node.children, depth + 1);
    }
    return html;
}

}

QString RenderEngine::render(const QVector<LogicalBlock> &blocks)
{
    QString html;

    auto closeAllLists = []() {};

    for(int i = 0; i < blocks.size(); ++i)
    {
        const LogicalBlock &block = blocks[i];
        switch(block.type)
        {
            // ==================== 列表 ====================
            case BlockType::UnorderedListItem:
            case BlockType::OrderedListItem:
            {
                int start = i;
                int end = start;
                while(end < blocks.size() && isListBlock(blocks[end]))
                    ++end;

                int treeIndex = start;
                QVector<ListNode> nodes = buildListTree(blocks, treeIndex, end, blocks[start].indentLevel);
                html += renderListTree(nodes, 0);
                i = end - 1;
                break;
            }

            // ==================== 段落 ====================
            case BlockType::Paragraph:
            {
                closeAllLists();
                html += "<p>";
                html += InlineParser::process(block.content);
                html += "</p>";
                break;
            }

            // ==================== 标题 ====================
            case BlockType::Heading1:
            case BlockType::Heading2:
            case BlockType::Heading3:
            case BlockType::Heading4:
            {
                closeAllLists();
                QString tag;
                switch(block.type)
                {
                case BlockType::Heading1: tag = "h1"; break;
                case BlockType::Heading2: tag = "h2"; break;
                case BlockType::Heading3: tag = "h3"; break;
                case BlockType::Heading4: tag = "h4"; break;
                default: break;
                }
                html += "<" + tag + ">";
                html += InlineParser::process(block.content);
                html += "</" + tag + ">";
                break;
            }

            // ==================== 代码块 ====================
            case BlockType::CodeBlockStart:
            {
                closeAllLists();
                html += "<table class=\"md-code-block\" width=\"100%\"><tr><td><pre>";
                break;
            }
            case BlockType::CodeBlockLine:
            {
                html += InlineParser::escapeHtml(block.content);
                html += "\n";
                break;
            }
            case BlockType::CodeBlockEnd:
            {
                html += "</pre></td></tr></table>";
                break;
            }

            // ==================== 水平线 ====================
            case BlockType::HorizontalRule:
            {
                closeAllLists();
                html += "<hr/>";
                break;
            }
            // ==================== 引用块 ====================
            case BlockType::Blockquote:
            {
                closeAllLists();
                html += "<blockquote>";
                html += InlineParser::process(block.content) ;
                html += "</blockquote>\n";
                break;
            }

        }
    }

    closeAllLists();

    return html;
}
