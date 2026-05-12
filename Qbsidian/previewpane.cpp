#include "previewpane.h"
#include "markdownparser.h"

PreviewPane::PreviewPane(QWidget *parent)
    : QTextBrowser(parent)
{
    setOpenExternalLinks(false);
}

void PreviewPane::showHtml(const QString &markdown)
{
    QString htmlBody = MarkdownParser::parse(markdown);
    QString fullHtml = QString(
        "<style>"
        "body{font-family:'SF Pro Text','Microsoft YaHei',sans-serif;font-size:14px;line-height:1.6;color:#dfe4f5;background-color:#292d3b;padding:16px;}"
        "h1,h2,h3,h4{color:#dfe4f5;border-bottom:1px solid #3b3f52;padding-bottom:6px;}"
        "h1{font-size:24px;}h2{font-size:20px;}h3{font-size:17px;}h4{font-size:15px;}"
        "p{margin:8px 0;}"
        "strong{color:#f5c2e7;}"
        "em{color:#f5e0dc;font-style:italic;}"
        "code{background:#3b3f52;padding:2px 6px;border-radius:4px;font-family:'SF Mono',Consolas,monospace;font-size:13px;color:#f9e2af;}"
        "pre{background:#222531;padding:12px;border-radius:8px;border:1px solid #3b3f52;overflow-x:auto;}"
        "pre code{background:transparent;padding:0;}"
        "ul,ol{padding-left:24px;}li{margin:4px 0;}"
        "hr{border:none;border-top:1px solid #52576d;margin:16px 0;}"
        "blockquote{border-left:3px solid #7c3aed;padding:4px 12px;margin:8px 0;color:#bcc3dc;background:#222531;border-radius:0 4px 4px 0;}"
        "table{border-collapse:collapse;width:100%;margin:12px 0;}"
        "th,td{border:1px solid #52576d;padding:6px 12px;text-align:left;}"
        "th{background:#3b3f52;}"
        "a{color:#7c3aed;text-decoration:none;}a:hover{text-decoration:underline;}"
        "</style>"
        "<div>%1</div>"
    ).arg(htmlBody);
    setHtml(fullHtml);
}
