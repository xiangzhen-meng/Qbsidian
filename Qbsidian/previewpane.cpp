 #include "previewpane.h"
#include "markdownparser.h"
#include "autohidescrollareafilter.h"
#include <QRegularExpression>
#include <QScrollBar>

PreviewPane::PreviewPane(QWidget *parent)
    : QTextBrowser(parent)
    , m_darkMode(false)
{
    setOpenLinks(false);
    setOpenExternalLinks(false);
    new AutoHideScrollAreaFilter(this, this);
}

void PreviewPane::setDarkMode(bool dark)
{
    m_darkMode = dark;
}

QString PreviewPane::buildStyleSheet() const
{
    if (m_darkMode) {
        return QStringLiteral(
            "body{font-family:'LXGW WenKai Screen','LXGW WenKai',-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Inter,Ubuntu,sans-serif;font-size:15px;line-height:1.62;color:#d8dee9;background-color:#2E3440;margin:0;}"
            "h1,h2,h3,h4,h5,h6{font-weight:700;line-height:1.28;}"
            "h1{font-size:1.85rem;margin-top:34px;margin-bottom:8px;color:#BF616A;}"
            "h2{font-size:1.55rem;margin-top:28px;margin-bottom:8px;color:#D08770;border-bottom:1px solid #4C566A;padding-bottom:5px;}"
            "h3{font-size:1.28rem;margin-top:22px;margin-bottom:4px;color:#EBCB8B;}"
            "h4{font-size:1.15rem;margin-top:18px;margin-bottom:2px;color:#A3BE8C;}"
            "h5{font-size:1.05rem;margin-top:16px;margin-bottom:2px;color:#88C0D0;}"
            "h6{font-size:0.95rem;margin-top:16px;margin-bottom:2px;color:#B48EAD;}"
            "p{margin:6px 0;}"
            "p.md-list-item{margin:3px 0;}"
            ".md-list-marker{color:#D08770;font-weight:700;}"
            "strong{font-weight:700;color:#eceff4;}"
            "em{font-style:italic;color:#A3BE8C;}"
            "code{color:#88C0D0;font-family:'JetBrains Mono','Fira Code',Menlo,SFMono-Regular,Consolas,monospace;font-size:14px;background-color:#3B4252;padding:3px 7px;}"
            ".md-blockquote{margin-top:10px;margin-bottom:10px;background-color:#3B4252;border-left:4px solid #81a1c1;}"
            ".md-blockquote td{border:none;color:#e5e9f0;font-style:italic;background-color:#3B4252;}"
            ".md-blockquote p{margin:4px 0;}"
            "hr{height:1px;border:none;background-color:#4C566A;margin:12px 0;}"
            "table{border-collapse:collapse;width:100%;margin:12px 0;background-color:#3B4252;}"
            "th,td{border:1px solid #4C566A;padding:7px 11px;text-align:left;}"
            "th{font-weight:700;color:#d8dee9;background:rgba(129,161,193,0.24);}"
            "tr:hover td{background:rgba(129,161,193,0.18);}"
            ".md-code-block{margin-top:12px;margin-bottom:12px;background-color:#3B4252;border:1px solid #4C566A;}"
            ".md-code-block td{border:none;background-color:#3B4252;padding:12px 15px;}"
            ".md-code-block pre{margin:0;color:#d8dee9;background-color:#3B4252;font-family:'JetBrains Mono','Fira Code',Menlo,SFMono-Regular,Consolas,monospace;font-size:14px;}"
            "td:first-child{font-weight:600;}"
            "a{color:#81a1c1;text-decoration:underline;}"
            "a:hover{color:#88C0D0;}"
            ".question-card{border:1px solid #4C566A;border-left:4px solid #A3BE8C;padding:11px 13px;margin:12px 0;border-radius:8px;background-color:#3B4252;}"
            ".question-card summary{font-weight:700;cursor:pointer;color:#d8dee9;outline:none;}"
            ".question-card .question-tag{color:#A3BE8C;font-size:.82em;border:1px solid #A3BE8C;border-radius:10px;padding:1px 7px;margin-right:6px;}"
            ".question-card .question-answer{margin-top:10px;color:#e5e9f0;padding-top:10px;border-top:1px dashed #4C566A;}"
        );
    } else {
        return QStringLiteral(
            "body{font-family:'LXGW WenKai Screen','LXGW WenKai',-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Inter,Ubuntu,sans-serif;font-size:15px;line-height:1.62;color:#0e0e0e;background-color:#ffffff;margin:0;}"
            "h1,h2,h3,h4,h5,h6{font-weight:700;line-height:1.28;}"
            "h1{font-size:1.85rem;margin-top:34px;margin-bottom:8px;color:#BF616A;}"
            "h2{font-size:1.55rem;margin-top:28px;margin-bottom:8px;color:#D08770;border-bottom:1px solid #dddddd;padding-bottom:5px;}"
            "h3{font-size:1.28rem;margin-top:22px;margin-bottom:4px;color:#e2b65e;}"
            "h4{font-size:1.15rem;margin-top:18px;margin-bottom:2px;color:#95b677;}"
            "h5{font-size:1.05rem;margin-top:16px;margin-bottom:2px;color:#65afc4;}"
            "h6{font-size:0.95rem;margin-top:16px;margin-bottom:2px;color:#B48EAD;}"
            "p{margin:6px 0;}"
            "p.md-list-item{margin:3px 0;}"
            ".md-list-marker{color:#D08770;font-weight:700;}"
            "strong{font-weight:700;color:#000000;}"
            "em{font-style:italic;color:#95b677;}"
            "code{color:#65afc4;font-family:'JetBrains Mono','Fira Code',Menlo,SFMono-Regular,Consolas,monospace;font-size:14px;background-color:#ECEFF4;padding:3px 7px;}"
            ".md-blockquote{margin-top:10px;margin-bottom:10px;background-color:#ECEFF4;border-left:4px solid #81a1c1;}"
            ".md-blockquote td{border:none;color:#0e0e0e;font-style:italic;background-color:#ECEFF4;}"
            ".md-blockquote p{margin:4px 0;}"
            "hr{height:1px;border:none;background-color:#dddddd;margin:12px 0;}"
            "table{border-collapse:collapse;width:100%;margin:12px 0;background:#f1f1f176;}"
            "th,td{border:1px solid #7d7d7d;padding:7px 11px;text-align:left;}"
            "th{font-weight:700;color:#0e0e0e;background:rgba(129,161,193,0.24);}"
            "tr:hover td{background:rgba(129,161,193,0.18);}"
            ".md-code-block{margin-top:12px;margin-bottom:12px;background-color:#ECEFF4;border:1px solid #d8dee9;}"
            ".md-code-block td{border:none;background-color:#ECEFF4;padding:12px 15px;}"
            ".md-code-block pre{margin:0;color:#0e0e0e;background-color:#ECEFF4;font-family:'JetBrains Mono','Fira Code',Menlo,SFMono-Regular,Consolas,monospace;font-size:14px;}"
            "td:first-child{font-weight:600;}"
            "a{color:#5e81ac;text-decoration:underline;}"
            "a:hover{color:#65afc4;}"
            ".question-card{border:1px solid #dddddd;border-left:4px solid #95b677;padding:11px 13px;margin:12px 0;border-radius:8px;background-color:#ECEFF4;}"
            ".question-card summary{font-weight:700;cursor:pointer;color:#0e0e0e;outline:none;}"
            ".question-card .question-tag{color:#95b677;font-size:.82em;border:1px solid #95b677;border-radius:10px;padding:1px 7px;margin-right:6px;}"
            ".question-card .question-answer{margin-top:10px;color:#0e0e0e;padding-top:10px;border-top:1px dashed #dddddd;}"
        );
    }
}

void PreviewPane::showHtml(const QString &markdown)
{
    int savedPos = verticalScrollBar()->value();

    //  Markdown 解析引擎正常解析文本
    QString htmlBody = MarkdownParser::parse(markdown);

    QRegularExpression regex("#题库/(\\S+)\\s+([^:<]+)::([^<]+)");

    QString htmlReplacement =
        "<div class=\"question-card\">"
        "  <details>"
        "    <summary>"
        "      <span class=\"question-tag\">\\1</span>"
        "      \\2"
        "    </summary>"
        "    <div class=\"question-answer\">"
        "      \\3"
        "    </div>"
        "  </details>"
        "</div>";

    htmlBody.replace(regex, htmlReplacement);

    QString fullHtml = QString(
                           "<style>%1</style>"
                           "<div style=\"margin:18px;\">%2</div>"
                           ).arg(buildStyleSheet(), htmlBody);

    setHtml(fullHtml);

    verticalScrollBar()->setValue(savedPos);
}
