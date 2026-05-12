#include "renderengine.h"
#include "inlineparser.h"

QString RenderEngine::render(const QVector<LogicalBlock> &blocks)
{
    QString html;

    struct RenderFrame
    {
        int indent;         // 该层列表的缩进值
        bool isOrdered;     // true=<ol>, false=<ul>
        bool itemOpen;      // 当前 <li> 是否已开未关
    };
    QVector<RenderFrame> renderStack;

    auto closeAllLists = [&]()
    {
        while(!renderStack.isEmpty())
        {
            if(renderStack.last().itemOpen)html += "</li>";
            html += renderStack.last().isOrdered ? "</ol>" : "</ul>";
            renderStack.pop_back();
        }
    };

    for(const LogicalBlock &block : blocks)
    {
        switch(block.type)
        {
            // ==================== 列表 ====================
            case BlockType::UnorderedListItem:
            case BlockType::OrderedListItem:
            {
                bool isOrdered = (block.type == BlockType::OrderedListItem);

                if(renderStack.isEmpty())
                {
                    renderStack.push_back({block.indentLevel, isOrdered, true});
                    html += isOrdered ? "<ol>" : "<ul>";
                    html += "<li>";
                }
                else if(block.indentLevel > renderStack.last().indent)
                {
                    renderStack.push_back({block.indentLevel, isOrdered, true});
                    html += isOrdered ? "<ol>" : "<ul>";
                    html += "<li>";
                }
                else if(block.indentLevel == renderStack.last().indent)
                {
                    // 关上一个 <li>
                    if(renderStack.last().itemOpen)
                        html += "</li>";

                    if(renderStack.last().isOrdered != isOrdered)
                    {
                        html += renderStack.last().isOrdered ? "</ol>" : "</ul>";
                        html += isOrdered ? "<ol>" : "<ul>";
                        renderStack.last().isOrdered = isOrdered;
                    }

                    html += "<li>";
                    renderStack.last().itemOpen = true;
                }
                else
                {
                    // 循环弹出更深的层级
                    while(!renderStack.isEmpty() && block.indentLevel < renderStack.last().indent)
                    {
                        if(renderStack.last().itemOpen)
                            html += "</li>";
                        html += renderStack.last().isOrdered ? "</ol>" : "</ul>";
                        renderStack.pop_back();
                    }

                    if(renderStack.isEmpty())
                    {
                        // 全部弹光，开新列表
                        renderStack.push_back({block.indentLevel, isOrdered, true});
                        html += isOrdered ? "<ol>" : "<ul>";
                        html += "<li>";
                    }
                    else if(block.indentLevel == renderStack.last().indent)
                    {
                        // 退到同级
                        if(renderStack.last().itemOpen)
                            html += "</li>";

                        // 检查类型切换
                        if(renderStack.last().isOrdered != isOrdered)
                        {
                            html += renderStack.last().isOrdered ? "</ol>" : "</ul>";
                            html += isOrdered ? "<ol>" : "</ul>";
                            renderStack.last().isOrdered = isOrdered;
                        }

                        html += "<li>";
                        renderStack.last().itemOpen = true;
                    }
                }

                // 输出正文
                html += InlineParser::process(block.content);
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
                html += "<pre><code>";
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
                html += "</code></pre>";
                break;
            }

            // ==================== 水平线 ====================
            case BlockType::HorizontalRule:
            {
                closeAllLists();
                html += "<hr/>";
                break;
            }
        }
    }

    closeAllLists();

    return html;
}
