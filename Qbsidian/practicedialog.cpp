#include "practicedialog.h"
#include "questionextractor.h"
#include "markdownparser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QScrollBar>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include <QDir>
#include <algorithm>
#include <random>

PracticeDialog::PracticeDialog(const QString &vaultPath, bool darkMode, QWidget *parent)
    : QDialog(parent)
    , m_vaultPath(vaultPath)
    , m_currentIndex(0)
    , m_answerVisible(false)
    , m_darkMode(darkMode)
    , m_questionBankMenu(nullptr)
{
    setWindowTitle(tr("抽查练习"));
    resize(760, 560);
    setMinimumSize(480, 360);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *infoBar = new QWidget(this);
    infoBar->setObjectName("infoBar");
    infoBar->setMinimumHeight(44);
    auto *infoLayout = new QHBoxLayout(infoBar);
    infoLayout->setContentsMargins(20, 10, 20, 10);

    m_progressLabel = new QLabel(this);
    m_progressLabel->setObjectName("progressLabel");

    m_sourceLabel = new QLabel(this);
    m_sourceLabel->setObjectName("sourceLabel");

    m_tagLabel = new QLabel(this);
    m_tagLabel->setObjectName("tagLabel");

    m_selectFolderButton = new QPushButton(tr("选择题库 ▾"), this);
    m_selectFolderButton->setObjectName("selectFolderButton");
    m_selectFolderButton->setCursor(Qt::PointingHandCursor);
    m_selectFolderButton->setFlat(true);

    m_questionBankMenu = new QMenu(m_selectFolderButton);
    QAction *presetAction = m_questionBankMenu->addAction(tr("默认题库"));
    QAction *folderAction = m_questionBankMenu->addAction(tr("选择文件夹..."));
    m_selectFolderButton->setMenu(m_questionBankMenu);

    infoLayout->addWidget(m_progressLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_sourceLabel);
    infoLayout->addSpacing(12);
    infoLayout->addWidget(m_tagLabel);
    infoLayout->addSpacing(12);
    infoLayout->addWidget(m_selectFolderButton);

    mainLayout->addWidget(infoBar);

    auto *sep = new QWidget(this);
    sep->setObjectName("infoSeparator");
    sep->setFixedHeight(1);
    mainLayout->addWidget(sep);

    m_browser = new QTextBrowser(this);
    m_browser->setOpenLinks(false);
    m_browser->setOpenExternalLinks(false);
    m_browser->setObjectName("practiceBrowser");
    m_browser->setFrameShape(QFrame::NoFrame);
    mainLayout->addWidget(m_browser, 1);

    auto *bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    auto *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(20, 14, 20, 14);

    m_showAnswerButton = new QPushButton(tr("查看答案"), this);
    m_showAnswerButton->setCursor(Qt::PointingHandCursor);
    m_showAnswerButton->setMinimumWidth(120);

    m_knownButton = new QPushButton(tr("认识"), this);
    m_knownButton->setCursor(Qt::PointingHandCursor);
    m_knownButton->setMinimumWidth(100);
    m_knownButton->hide();

    m_unknownButton = new QPushButton(tr("不认识"), this);
    m_unknownButton->setCursor(Qt::PointingHandCursor);
    m_unknownButton->setFlat(true);
    m_unknownButton->setMinimumWidth(100);
    m_unknownButton->hide();

    bottomLayout->addStretch();
    bottomLayout->addWidget(m_unknownButton);
    bottomLayout->addSpacing(12);
    bottomLayout->addWidget(m_knownButton);
    bottomLayout->addSpacing(12);
    bottomLayout->addWidget(m_showAnswerButton);
    bottomLayout->addStretch();

    mainLayout->addWidget(bottomBar);

    connect(m_showAnswerButton, &QPushButton::clicked, this, &PracticeDialog::onShowAnswer);
    connect(m_knownButton, &QPushButton::clicked, this, &PracticeDialog::onKnown);
    connect(m_unknownButton, &QPushButton::clicked, this, &PracticeDialog::onUnknown);
    connect(presetAction, &QAction::triggered, this, &PracticeDialog::onSelectPreset);
    connect(folderAction, &QAction::triggered, this, &PracticeDialog::onSelectFolder);

    setStyleSheet(buildStyleSheet());

    loadPresetQuestions(false);
}

void PracticeDialog::showCurrentQuestion()
{
    if (m_questions.isEmpty() || m_currentIndex >= m_questions.size())
        return;

    const ExtractedQuestion &q = m_questions[m_currentIndex];

    m_progressLabel->setText(tr("第 %1 / %2 题").arg(m_currentIndex + 1).arg(m_questions.size()));

    QString sourceName;
    if (q.sourceFile == QStringLiteral("System Preset"))
        sourceName = tr("系统预设题库");
    else
        sourceName = QFileInfo(q.sourceFile).fileName();
    m_sourceLabel->setText(sourceName);
    m_sourceLabel->setToolTip(q.sourceFile);

    if (q.tag.isEmpty())
        m_tagLabel->hide();
    else {
        m_tagLabel->setText(q.tag);
        m_tagLabel->show();
    }

    m_answerVisible = false;
    m_showAnswerButton->show();
    m_knownButton->hide();
    m_unknownButton->hide();

    m_browser->setHtml(renderQuestionHtml(q, false));
    m_browser->verticalScrollBar()->setValue(0);
}

void PracticeDialog::onShowAnswer()
{
    if (m_currentIndex >= m_questions.size())
        return;

    m_answerVisible = true;
    m_showAnswerButton->hide();
    m_knownButton->show();
    m_unknownButton->show();

    const ExtractedQuestion &q = m_questions[m_currentIndex];
    m_browser->setHtml(renderQuestionHtml(q, true));
}

void PracticeDialog::onKnown()
{
    advanceQuestion();
}

void PracticeDialog::onUnknown()
{
    advanceQuestion();
}

void PracticeDialog::onSelectPreset()
{
    loadPresetQuestions(true);
}

void PracticeDialog::advanceQuestion()
{
    if (!m_answerVisible)
        return;

    m_currentIndex++;
    if (m_currentIndex >= m_questions.size()) {
        QMessageBox::information(this, tr("练习完成"),
                                 tr("恭喜完成今日练习！"));
        accept();
        return;
    }

    showCurrentQuestion();
}

void PracticeDialog::onSelectFolder()
{
    QString startDir = m_currentDirectory.isEmpty() ? m_vaultPath : m_currentDirectory;
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择题库文件夹"),
        startDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    loadQuestionsFromDirectory(dir, true);
}

bool PracticeDialog::loadPresetQuestions(bool showEmptyMessage)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QuestionExtractor extractor;
    QList<ExtractedQuestion> allQuestions = extractor.extractFromPreset();

    QApplication::restoreOverrideCursor();

    if (allQuestions.isEmpty()) {
        if (showEmptyMessage)
            QMessageBox::information(this, tr("提示"), tr("默认题库为空。"));
        else
            QMessageBox::information(this, tr("提示"), tr("当前题库为空。"));
        return false;
    }

    applyShuffleAndLimit(allQuestions);

    m_currentDirectory.clear();
    m_questions = allQuestions;
    m_currentIndex = 0;
    m_answerVisible = false;
    showCurrentQuestion();

    return true;
}

bool PracticeDialog::loadQuestionsFromDirectory(const QString &directory, bool showEmptyMessage)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QuestionExtractor extractor;
    QList<ExtractedQuestion> allQuestions;

    allQuestions.append(extractor.extractAllFromDirectory(directory));

    QApplication::restoreOverrideCursor();

    if (allQuestions.isEmpty()) {
        if (showEmptyMessage)
            QMessageBox::information(this, tr("提示"), tr("所选文件夹没有题目。"));
        else
            QMessageBox::information(this, tr("提示"), tr("当前题库为空，请先在笔记中添加题目！"));
        return false;
    }

    applyShuffleAndLimit(allQuestions);

    m_currentDirectory = directory;
    m_questions = allQuestions;
    m_currentIndex = 0;
    m_answerVisible = false;
    showCurrentQuestion();

    return true;
}

void PracticeDialog::applyShuffleAndLimit(QList<ExtractedQuestion> &questions)
{
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(questions.begin(), questions.end(), g);
    }

    if (questions.size() > 10)
        questions = questions.mid(0, 10);
}

QString PracticeDialog::renderQuestionHtml(const ExtractedQuestion &question, bool showAnswer) const
{
    QString questionHtml = MarkdownParser::parse(question.question);

    QString html = QStringLiteral(
                       "<style>%1</style>"
                       "<div class=\"question-content\">%2</div>")
                       .arg(buildContentStyleSheet(), questionHtml);

    if (showAnswer) {
        QString answerHtml = MarkdownParser::parse(question.answer);
        html += QStringLiteral(
            "<div class=\"answer-section\">"
            "  <div class=\"answer-divider\"></div>"
            "  <div class=\"answer-content\">%1</div>"
            "</div>")
            .arg(answerHtml);
    }

    return html;
}

QString PracticeDialog::buildStyleSheet() const
{
    if (m_darkMode) {
        return QStringLiteral(
            "QDialog{background-color:#2E3440;}"
            "QWidget{font-family:\"LXGW WenKai Screen\",\"LXGW WenKai\",-apple-system,BlinkMacSystemFont,\"Segoe UI\",Helvetica,Arial,sans-serif;font-size:14px;color:#d8dee9;}"
            "QTextBrowser{background-color:#2E3440;color:#d8dee9;border:none;font-size:18px;padding:24px;}"
            "QScrollBar:vertical{width:5px;border:none;background:transparent;}"
            "QScrollBar::handle:vertical{background:rgba(216,222,233,0.26);border-radius:2px;min-height:30px;}"
            "QScrollBar::handle:vertical:hover{background:rgba(216,222,233,0.42);}"
            "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
            "QScrollBar:horizontal{height:5px;border:none;background:transparent;}"
            "QScrollBar::handle:horizontal{background:rgba(216,222,233,0.26);border-radius:2px;min-width:30px;}"
            "QScrollBar::handle:horizontal:hover{background:rgba(216,222,233,0.42);}"
            "QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}"
            "QPushButton{background-color:#81a1c1;color:#2E3440;border:none;border-radius:6px;padding:7px 16px;font-weight:600;}"
            "QPushButton:hover{background-color:#88C0D0;}"
            "QPushButton:pressed{background-color:#5e81ac;color:#eceff4;}"
            "QPushButton:flat{background:transparent;color:#88C0D0;}"
            "QPushButton:flat:hover{background:rgba(129,161,193,0.14);}"
            "QPushButton:checked{background-color:#81a1c1;color:#2E3440;}"
            "#infoBar{background-color:#3B4252;}"
            "#infoSeparator{background-color:#4C566A;}"
            "#progressLabel{color:#e5e9f0;font-size:13px;}"
            "#sourceLabel{color:#d8dee9;font-size:13px;}"
            "#tagLabel{color:#81a1c1;font-size:12px;border:1px solid #81a1c1;border-radius:10px;padding:2px 10px;}"
            "#selectFolderButton{color:#88C0D0;font-size:12px;text-decoration:underline;}"
            "#selectFolderButton::menu-indicator{image:none;width:0px;}"
            "#bottomBar{background-color:#3B4252;}"
        );
    } else {
        return QStringLiteral(
            "QDialog{background-color:#ffffff;}"
            "QWidget{font-family:\"LXGW WenKai Screen\",\"LXGW WenKai\",-apple-system,BlinkMacSystemFont,\"Segoe UI\",Helvetica,Arial,sans-serif;font-size:14px;color:#0e0e0e;}"
            "QTextBrowser{background-color:#ffffff;color:#0e0e0e;border:none;font-size:18px;padding:24px;}"
            "QScrollBar:vertical{width:5px;border:none;background:transparent;}"
            "QScrollBar::handle:vertical{background:#dcdcdc;border-radius:2px;min-height:30px;}"
            "QScrollBar::handle:vertical:hover{background:#c8d4e0;}"
            "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
            "QScrollBar:horizontal{height:5px;border:none;background:transparent;}"
            "QScrollBar::handle:horizontal{background:#dcdcdc;border-radius:2px;min-width:30px;}"
            "QScrollBar::handle:horizontal:hover{background:#c8d4e0;}"
            "QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}"
            "QPushButton{background-color:#81a1c1;color:#ffffff;border:none;border-radius:6px;padding:7px 16px;font-weight:600;}"
            "QPushButton:hover{background-color:#5e81ac;}"
            "QPushButton:pressed{background-color:#4c6f99;}"
            "QPushButton:flat{background:transparent;color:#5e81ac;}"
            "QPushButton:flat:hover{background:rgba(129,161,193,0.12);}"
            "QPushButton:checked{background-color:#81a1c1;color:#ffffff;}"
            "#infoBar{background-color:#fcfcfc;}"
            "#infoSeparator{background-color:#dddddd;}"
            "#progressLabel{color:#7f7f7f;font-size:13px;}"
            "#sourceLabel{color:#0e0e0e;font-size:13px;}"
            "#tagLabel{color:#81a1c1;font-size:12px;border:1px solid #81a1c1;border-radius:10px;padding:2px 10px;}"
            "#selectFolderButton{color:#5e81ac;font-size:12px;text-decoration:underline;}"
            "#selectFolderButton::menu-indicator{image:none;width:0px;}"
            "#bottomBar{background-color:#fcfcfc;}"
        );
    }
}

QString PracticeDialog::buildContentStyleSheet() const
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
            ".answer-section{margin-top:24px;}"
            ".answer-divider{border-top:2px dashed #4C566A;margin-bottom:16px;}"
            ".answer-content{}"
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
            ".answer-section{margin-top:24px;}"
            ".answer-divider{border-top:2px dashed #dddddd;margin-bottom:16px;}"
            ".answer-content{}"
        );
    }
}
