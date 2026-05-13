#include "inlineparser.h"
#include <QRegularExpression>

QString InlineParser::process(const QString &text)
{
    QString html = text;
    html.replace("&", "&amp;");
    html.replace("<", "&lt;");
    html.replace(">", "&gt;");

    //图片
    QRegularExpression imgRe("!\\[(.*?)\\]\\((.*?)\\)");
    html.replace(imgRe, "<img src=\"\\2\" alt=\"\\1\" style=\"max-width:100%;\" />");
    // 超链接
    QRegularExpression linkRe("\\[(.*?)\\]\\((.*?)\\)");
    html.replace(linkRe, "<a href=\"\\2\">\\1</a>");
    QRegularExpression autolinkRe("<(https?://.+?)>");
    html.replace(autolinkRe, "<a href=\"\\1\">\\1</a>");
    //行内代码块
    QRegularExpression codeRe("`(.+?)`");
    html.replace(codeRe, "<code>\\1</code>");
    //粗体
    QRegularExpression boldRe("\\*\\*(.+?)\\*\\*",QRegularExpression::DotMatchesEverythingOption);
    html.replace(boldRe, "<strong>\\1</strong>");
    //斜体
    QRegularExpression italicRe("\\*(.+?)\\*",QRegularExpression::DotMatchesEverythingOption);
    html.replace(italicRe, "<em>\\1</em>");
    // 删除线
    QRegularExpression strikeRe("~~(.+?)~~", QRegularExpression::DotMatchesEverythingOption);
    html.replace(strikeRe, "<del>\\1</del>");


    return html;
}
QString InlineParser::escapeHtml(const QString &text)
{
    QString result = text;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    return result;
}
