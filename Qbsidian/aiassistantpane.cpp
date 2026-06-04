#include "aiassistantpane.h"
#include "autohidescrollareafilter.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

AIAssistantPane::AIAssistantPane(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_inputEdit(nullptr)
    , m_sendButton(nullptr)
    , m_summaryButton(nullptr)
    , m_quizButton(nullptr)
    , m_settingsButton(nullptr)
    , m_closeButton(nullptr)
    , m_titleLabel(nullptr)
    , m_darkMode(false)
{
    setObjectName("aiAssistantPane");
    setMinimumWidth(380);

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QWidget *header = new QWidget(this);
    header->setObjectName("aiHeader");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 10, 10, 10);
    headerLayout->setSpacing(8);

    QLabel *avatar = new QLabel(header);
    avatar->setObjectName("aiHeaderAvatar");
    avatar->setPixmap(QIcon(":/icons/ai-robot.svg").pixmap(22, 22));
    avatar->setFixedSize(28, 28);
    avatar->setAlignment(Qt::AlignCenter);

    m_titleLabel = new QLabel(tr("AI 助教"), header);
    m_titleLabel->setObjectName("aiTitle");

    m_settingsButton = new QPushButton(header);
    m_settingsButton->setObjectName("aiSettingsButton");
    m_settingsButton->setFixedSize(26, 26);
    m_settingsButton->setIcon(QIcon(":/icons/ai-settings.svg"));
    m_settingsButton->setIconSize(QSize(18, 18));
    m_settingsButton->setFlat(true);
    m_settingsButton->setToolTip(tr("设置 API Key"));

    m_closeButton = new QPushButton(tr("×"), header);
    m_closeButton->setObjectName("aiCloseButton");
    m_closeButton->setFixedSize(26, 26);
    m_closeButton->setFlat(true);
    m_closeButton->setToolTip(tr("关闭 AI 助教"));

    headerLayout->addWidget(avatar);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_settingsButton);
    headerLayout->addStretch();
    headerLayout->addWidget(m_closeButton);
    rootLayout->addWidget(header);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("aiMessageScroll");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_messageContainer = new QWidget(m_scrollArea);
    m_messageContainer->setObjectName("aiMessageContainer");
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(12, 14, 12, 14);
    m_messageLayout->setSpacing(12);
    m_messageLayout->addStretch();
    m_scrollArea->setWidget(m_messageContainer);
    rootLayout->addWidget(m_scrollArea, 1);

    QWidget *actionBar = new QWidget(this);
    actionBar->setObjectName("aiActionBar");
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setContentsMargins(10, 8, 10, 6);
    actionLayout->setSpacing(8);
    m_summaryButton = new QPushButton(tr("快捷总结"), actionBar);
    m_quizButton = new QPushButton(tr("给我出题"), actionBar);
    m_summaryButton->setObjectName("aiSecondaryButton");
    m_quizButton->setObjectName("aiSecondaryButton");
    actionLayout->addWidget(m_summaryButton);
    actionLayout->addWidget(m_quizButton);
    rootLayout->addWidget(actionBar);

    QWidget *inputPanel = new QWidget(this);
    inputPanel->setObjectName("aiInputPanel");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputPanel);
    inputLayout->setContentsMargins(10, 0, 10, 10);
    inputLayout->setSpacing(8);
    m_inputEdit = new QPlainTextEdit(inputPanel);
    m_inputEdit->setObjectName("aiInputEdit");
    m_inputEdit->setPlaceholderText(tr("输入问题，若当前选中内容会自动引用..."));
    m_inputEdit->setFixedHeight(78);
    m_sendButton = new QPushButton(tr("发送"), inputPanel);
    m_sendButton->setObjectName("aiSendButton");
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_sendButton);
    rootLayout->addWidget(inputPanel);

    connect(m_closeButton, &QPushButton::clicked, this, &AIAssistantPane::closeRequested);
    connect(m_settingsButton, &QPushButton::clicked, this, &AIAssistantPane::settingsRequested);
    connect(m_summaryButton, &QPushButton::clicked, this, &AIAssistantPane::summarizeRequested);
    connect(m_quizButton, &QPushButton::clicked, this, &AIAssistantPane::quizRequested);
    connect(m_sendButton, &QPushButton::clicked, this, [this]() {
        QString text = m_inputEdit->toPlainText().trimmed();
        if (text.isEmpty())
            return;
        m_inputEdit->clear();
        emit questionSubmitted(text);
    });

    new AutoHideScrollAreaFilter(m_scrollArea, this);
    new AutoHideScrollAreaFilter(m_inputEdit, this);
    updateTheme();
}

void AIAssistantPane::appendMessage(AiMessageRole role, const QString &text, bool quoted)
{
    int stretchIndex = m_messageLayout->count() - 1;
    QWidget *row = new QWidget(m_messageContainer);
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);

    if (role == AiMessageRole::System) {
        rowLayout->addStretch();
        rowLayout->addWidget(createBubble(role, text, quoted));
        rowLayout->addStretch();
    } else if (role == AiMessageRole::Ai) {
        rowLayout->addWidget(createAvatar(role), 0, Qt::AlignTop);
        rowLayout->addWidget(createBubble(role, text, quoted));
        rowLayout->addStretch();
    } else {
        rowLayout->addStretch();
        rowLayout->addWidget(createBubble(role, text, quoted));
        rowLayout->addWidget(createAvatar(role), 0, Qt::AlignTop);
    }

    m_messageLayout->insertWidget(stretchIndex, row);
    updateBubbleWidths();
    scrollToBottom();
}

void AIAssistantPane::appendUserMessage(const QString &text, bool quoted)
{
    appendMessage(AiMessageRole::User, text, quoted);
}

void AIAssistantPane::appendAiMessage(const QString &text)
{
    appendMessage(AiMessageRole::Ai, text);
}

void AIAssistantPane::appendSystemMessage(const QString &text)
{
    appendMessage(AiMessageRole::System, text);
}

void AIAssistantPane::setBusy(bool busy)
{
    m_sendButton->setEnabled(!busy);
    m_summaryButton->setEnabled(!busy);
    m_quizButton->setEnabled(!busy);
}

void AIAssistantPane::setDarkMode(bool dark)
{
    m_darkMode = dark;
    updateTheme();
}

QWidget *AIAssistantPane::createAvatar(AiMessageRole role)
{
    QLabel *avatar = new QLabel(m_messageContainer);
    avatar->setFixedSize(30, 30);
    avatar->setAlignment(Qt::AlignCenter);
    if (role == AiMessageRole::Ai) {
        avatar->setObjectName("aiChatAvatar");
        avatar->setPixmap(QIcon(":/icons/ai-robot.svg").pixmap(22, 22));
    } else {
        avatar->setObjectName("userChatAvatar");
        avatar->setText(tr("我"));
    }
    avatar->setStyleSheet(m_darkMode
        ? "QLabel#aiChatAvatar { background: #3B4252; border-radius: 15px; } QLabel#userChatAvatar { background: #81a1c1; color: #2E3440; border-radius: 15px; font-weight: 700; }"
        : "QLabel#aiChatAvatar { background: #f3f3f3; border-radius: 15px; } QLabel#userChatAvatar { background: #81a1c1; color: #ffffff; border-radius: 15px; font-weight: 700; }");
    return avatar;
}

QWidget *AIAssistantPane::createBubble(AiMessageRole role, const QString &text, bool quoted)
{
    QLabel *bubble = new QLabel(m_messageContainer);
    bubble->setWordWrap(true);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    QString display = text.toHtmlEscaped().replace('\n', "<br>");
    if (quoted)
        display += tr("<br><span style=\"font-size:12px; opacity:.72;\">已引用选中内容</span>");
    bubble->setText(display);
    m_bubbles.append(bubble);
    bubble->setMinimumWidth(role == AiMessageRole::System ? 120 : 40);
    bubble->setContentsMargins(10, 8, 10, 8);

    QString style;
    if (role == AiMessageRole::Ai) {
        style = m_darkMode
            ? "QLabel { background: #3B4252; color: #d8dee9; border-radius: 12px; padding: 9px 11px; }"
            : "QLabel { background: #f3f3f3; color: #0e0e0e; border-radius: 12px; padding: 9px 11px; }";
    } else if (role == AiMessageRole::User) {
        style = m_darkMode
            ? "QLabel { background: #81a1c1; color: #2E3440; border-radius: 12px; padding: 9px 11px; }"
            : "QLabel { background: #81a1c1; color: #ffffff; border-radius: 12px; padding: 9px 11px; }";
    } else {
        style = m_darkMode
            ? "QLabel { background: rgba(216,222,233,0.10); color: #d8dee9; border-radius: 10px; padding: 6px 10px; font-size: 12px; }"
            : "QLabel { background: rgba(129,161,193,0.14); color: #7f7f7f; border-radius: 10px; padding: 6px 10px; font-size: 12px; }";
    }
    bubble->setStyleSheet(style);
    return bubble;
}

void AIAssistantPane::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBubbleWidths();
}

void AIAssistantPane::updateBubbleWidths()
{
    int bubbleWidth = qMax(260, width() - 88);
    for (QLabel *bubble : m_bubbles) {
        if (bubble)
            bubble->setFixedWidth(bubbleWidth);
    }
}

void AIAssistantPane::scrollToBottom()
{
    QTimer::singleShot(0, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
    });
}

void AIAssistantPane::updateTheme()
{
    setStyleSheet(m_darkMode
        ? QStringLiteral(
            "QWidget#aiAssistantPane { background: #2E3440; border-left: 1px solid #2E3440; }"
            "QWidget#aiHeader, QWidget#aiActionBar, QWidget#aiInputPanel { background: #2E3440; }"
            "QLabel#aiTitle { color: #eceff4; font-weight: 700; font-size: 15px; }"
            "QPushButton#aiSettingsButton { background: transparent; border: none; border-radius: 13px; }"
            "QPushButton#aiSettingsButton:hover { background: rgba(216,222,233,0.14); }"
            "QPushButton#aiCloseButton { background: transparent; color: #d8dee9; border: none; border-radius: 13px; font-size: 18px; }"
            "QPushButton#aiCloseButton:hover { background: rgba(216,222,233,0.14); }"
            "QScrollArea#aiMessageScroll, QWidget#aiMessageContainer { background: #2E3440; border: none; }"
            "QPlainTextEdit#aiInputEdit { background: #3B4252; color: #d8dee9; border: 1px solid #4C566A; border-radius: 8px; padding: 8px; font-size: 14px; }"
            "QPlainTextEdit#aiInputEdit:focus { border-color: #81a1c1; }"
            "QPushButton#aiSecondaryButton { padding: 7px 10px; }"
            "QPushButton#aiSendButton { padding: 8px 12px; }")
        : QStringLiteral(
            "QWidget#aiAssistantPane { background: #ffffff; border-left: 1px solid #dddddd; }"
            "QWidget#aiHeader, QWidget#aiActionBar, QWidget#aiInputPanel { background: #ffffff; }"
            "QLabel#aiTitle { color: #0e0e0e; font-weight: 700; font-size: 15px; }"
            "QPushButton#aiSettingsButton { background: transparent; border: none; border-radius: 13px; }"
            "QPushButton#aiSettingsButton:hover { background: rgba(127,127,127,0.16); }"
            "QPushButton#aiCloseButton { background: transparent; color: #7f7f7f; border: none; border-radius: 13px; font-size: 18px; }"
            "QPushButton#aiCloseButton:hover { background: rgba(127,127,127,0.16); }"
            "QScrollArea#aiMessageScroll, QWidget#aiMessageContainer { background: #ffffff; border: none; }"
            "QPlainTextEdit#aiInputEdit { background: #fcfcfc; color: #0e0e0e; border: 1px solid #dddddd; border-radius: 8px; padding: 8px; font-size: 14px; }"
            "QPlainTextEdit#aiInputEdit:focus { border-color: #81a1c1; }"
            "QPushButton#aiSecondaryButton { padding: 7px 10px; }"
            "QPushButton#aiSendButton { padding: 8px 12px; }")
    );
}
