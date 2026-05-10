#include "markdownparser.h"
#include "blockparser.h"
#include "renderengine.h"

QString MarkdownParser::parse(const QString &markdown)
{
    // 1. 换行符归一化：\r\n → \n，\r → \n
    QString normalized = markdown;
    normalized.replace("\r\n", "\n").replace("\r", "\n");

    // 2. BlockParser：逐行扫描，生成块数组
    //    这里LogicalBlock 的 content 是原始纯文本
    QVector<LogicalBlock> blocks = BlockParser::parseBlocks(normalized);

    // 3. RenderEngine：遍历 blocks，维护列表栈，对每个 content 调 InlineParser，拼接 HTML
    QString htmlBody = RenderEngine::render(blocks);

    return htmlBody;
}
