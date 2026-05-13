#include "previewpane.h"
#include "markdownparser.h"

PreviewPane::PreviewPane(QWidget *parent)
    : QTextBrowser(parent)
    , m_darkMode(false)
{
    setOpenExternalLinks(false);
}

void PreviewPane::setDarkMode(bool dark)
{
    m_darkMode = dark;
}

QString PreviewPane::buildStyleSheet() const
{
    if (m_darkMode) {
        return QStringLiteral(
            "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Inter,Ubuntu,sans-serif;font-size:15px;line-height:1.5;color:#dadada;background-color:#1c2127;padding:16px;}"
            "h1{font-size:1.7rem;font-weight:600;margin-top:32px;margin-bottom:4px;color:#dadada;}"
            "h2{font-size:1.5rem;font-weight:600;margin-top:23px;margin-bottom:8px;color:#dadada;border-bottom:2px solid #35393e;padding-bottom:2px;}"
            "h3{font-size:1.2rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#3875ab;}"
            "h4{font-size:1.1rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#c0be78;}"
            "h5{font-size:1.0rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#d4656a;}"
            "h6{font-size:0.9rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#999;}"
            "p{margin:4px 0;}"
            "p.md-list-item{margin:3px 0;}"
            ".md-list-marker{color:#999;}"
            "strong{font-weight:600;color:#d8a0b8;}"
            "em{font-style:italic;color:#d8a0b8;}"
            "code{color:#dadada;font-family:'JetBrains Mono','Fira Code',Menlo,SFMono-Regular,Consolas,monospace;font-size:14px;background:#303540;padding:3px 6px;border-radius:4px;}"
            "blockquote{border-left:3px solid #35393e;padding:2px 14px;margin:4px 0;color:#62b97b;font-style:italic;background:transparent;}"
            "hr{height:1px;border:none;background-color:#35393e;margin:8px 0;}"
            "table{border-collapse:collapse;width:100%;margin:8px 0;}"
            "th,td{border:1px solid #35393e;padding:6px 10px;text-align:left;}"
            ".md-code-block{width:100%;border-collapse:separate;border-spacing:0;margin-top:10px;margin-bottom:10px;background:#282c34;border-radius:4px;}"
            ".md-code-block td{padding:10px 14px;border:none;background:transparent;}"
            ".md-code-block pre{margin:0;background:transparent;border:none;overflow-x:auto;padding:0;}"
            "th{font-weight:500;color:#999;background:transparent;}"
            "td:first-child{font-weight:600;}"
            "a{color:#2e80f2;text-decoration:underline;}"
            "a:hover{color:#5a9ff5;}"
        );
    } else {
        return QStringLiteral(
            "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Inter,Ubuntu,sans-serif;font-size:15px;line-height:1.5;color:#222222;background-color:#ffffff;padding:16px;}"
            "h1{font-size:1.7rem;font-weight:600;margin-top:32px;margin-bottom:4px;color:#222222;}"
            "h2{font-size:1.5rem;font-weight:600;margin-top:23px;margin-bottom:8px;color:#222222;border-bottom:2px solid #ebedf0;padding-bottom:2px;}"
            "h3{font-size:1.2rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#31738b;}"
            "h4{font-size:1.1rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#a38544;}"
            "h5{font-size:1.0rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#be5058;}"
            "h6{font-size:0.9rem;font-weight:600;margin-top:16px;margin-bottom:0;color:#ababab;}"
            "p{margin:4px 0;}"
            "p.md-list-item{margin:3px 0;}"
            ".md-list-marker{color:#ababab;}"
            "strong{font-weight:600;color:#c88090;}"
            "em{font-style:italic;color:#c88090;}"
            "code{color:#222222;font-family:'JetBrains Mono','Fira Code',Menlo,SFMono-Regular,Consolas,monospace;font-size:14px;background:#eceef1;padding:3px 6px;border-radius:4px;}"
            "blockquote{border-left:3px solid #ebedf0;padding:2px 14px;margin:4px 0;color:#3b8860;font-style:italic;background:transparent;}"
            "hr{height:1px;border:none;background-color:#ebedf0;margin:8px 0;}"
            "table{border-collapse:collapse;width:100%;margin:8px 0;}"
            "th,td{border:1px solid #ebedf0;padding:6px 10px;text-align:left;}"
            ".md-code-block{width:100%;border-collapse:separate;border-spacing:0;margin-top:10px;margin-bottom:10px;background:#f0f2f5;border-radius:4px;}"
            ".md-code-block td{padding:10px 14px;border:none;background:transparent;}"
            ".md-code-block pre{margin:0;background:transparent;border:none;overflow-x:auto;padding:0;}"
            "th{font-weight:500;color:#ababab;background:transparent;}"
            "td:first-child{font-weight:600;}"
            "a{color:#2e80f2;text-decoration:underline;}"
            "a:hover{color:#1a6fd6;}"
        );
    }
}

void PreviewPane::showHtml(const QString &markdown)
{
    QString htmlBody = MarkdownParser::parse(markdown);
    QString fullHtml = QString(
        "<style>%1</style>"
        "<div>%2</div>"
    ).arg(buildStyleSheet(), htmlBody);
    setHtml(fullHtml);
}
