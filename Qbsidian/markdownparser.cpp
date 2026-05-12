#include "markdownparser.h"
#include "blockparser.h"
#include "renderengine.h"

QString MarkdownParser::parse(const QString &markdown)
{
    // 换行符统一：\r\n → \n，\r → \n
    QString normalized = markdown;
    normalized.replace("\r\n", "\n").replace("\r", "\n");

    // BlockParser：逐行扫描，文本分块
    QVector<LogicalBlock> blocks = BlockParser::parseBlocks(normalized);

    // RenderEngine：遍历 blocks，拼接 HTML
    QString htmlBody = RenderEngine::render(blocks);

    return htmlBody;
}
