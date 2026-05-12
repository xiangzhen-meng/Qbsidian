#include "inlineparser.h"
#include <QRegularExpression>

QString InlineParser::process(const QString &text)
{
    QString html = text;
    html.replace("&", "&amp;");
    html.replace("<", "&lt;");
    html.replace(">", "&gt;");

    QRegularExpression codeRe("`(.+?)`");
    html.replace(codeRe, "<code>\\1</code>");

    QRegularExpression boldRe("\\*\\*(.+?)\\*\\*");
    html.replace(boldRe, "<strong>\\1</strong>");

    QRegularExpression italicRe("\\*(.+?)\\*");
    html.replace(italicRe, "<em>\\1</em>");
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
