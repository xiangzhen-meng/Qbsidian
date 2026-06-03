#include "reviewtimelinepane.h"
#include "previewpane.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QMimeData>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QScrollBar>

ReviewNoteButton::ReviewNoteButton(const ReviewPlanItem &item, QWidget *parent)
    : QPushButton(item.noteTitle.isEmpty() ? QFileInfo(item.noteId).completeBaseName() : item.noteTitle, parent)
    , m_item(item)
{
}

void ReviewNoteButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit doubleClicked(m_item);
    QPushButton::mouseDoubleClickEvent(event);
}

ReviewTimelineNode::ReviewTimelineNode(const QDate &date, QWidget *parent)
    : QWidget(parent)
    , m_date(date)
    , m_rootLayout(nullptr)
    , m_notesLayout(nullptr)
    , m_label(nullptr)
    , m_dot(nullptr)
    , m_reviewCount(0)
{
    setAcceptDrops(true);
    setFixedWidth(132);
    setObjectName("timelineDayColumn");

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(6, 8, 6, 6);
    m_rootLayout->setSpacing(6);

    QWidget *notesArea = new QWidget(this);
    notesArea->setObjectName("timelineNotesArea");
    QVBoxLayout *notesAreaLayout = new QVBoxLayout(notesArea);
    notesAreaLayout->setContentsMargins(0, 0, 0, 0);
    notesAreaLayout->setSpacing(0);

    m_notesLayout = new QVBoxLayout;
    m_notesLayout->setContentsMargins(0, 0, 0, 0);
    m_notesLayout->setSpacing(16);
    notesAreaLayout->addLayout(m_notesLayout, 1);

    m_label = new QLabel(dateLabel(), this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setObjectName("timelineNodeLabel");

    m_dot = new QLabel(QStringLiteral("●"), this);
    m_dot->setAlignment(Qt::AlignCenter);
    m_dot->setObjectName("timelineNodeDot");

    m_rootLayout->addWidget(notesArea, 1);
    m_rootLayout->addWidget(m_label);
    m_rootLayout->addWidget(m_dot);

    setDropHighlighted(false);
    setHasReviews(false);
}

QDate ReviewTimelineNode::date() const
{
    return m_date;
}

void ReviewTimelineNode::setReviews(const QList<ReviewPlanItem> &reviews)
{
    m_reviewCount = reviews.size();
    while (QLayoutItem *item = m_notesLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    m_notesLayout->addStretch(1);
    for (const ReviewPlanItem &item : reviews) {
        ReviewNoteButton *noteButton = new ReviewNoteButton(item, this);
        noteButton->setObjectName("timelineNoteButton");
        noteButton->setToolTip(item.noteId);
        noteButton->setCursor(Qt::PointingHandCursor);
        noteButton->setFixedHeight(36);
        noteButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        connect(noteButton, &QPushButton::clicked, this, [this, item]() {
            emit notePreviewRequested(item);
        });
        connect(noteButton, &ReviewNoteButton::doubleClicked, this, [this](const ReviewPlanItem &clickedItem) {
            emit noteOpenRequested(clickedItem.noteId);
        });
        m_notesLayout->addWidget(noteButton);
    }

    setHasReviews(!reviews.isEmpty());
}

int ReviewTimelineNode::reviewCount() const
{
    return m_reviewCount;
}

void ReviewTimelineNode::dragEnterEvent(QDragEnterEvent *event)
{
    if (!draggedNotePath(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        setDropHighlighted(true);
        return;
    }
    event->ignore();
}

void ReviewTimelineNode::dragMoveEvent(QDragMoveEvent *event)
{
    if (!draggedNotePath(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        return;
    }
    event->ignore();
}

void ReviewTimelineNode::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    setDropHighlighted(false);
}

void ReviewTimelineNode::dropEvent(QDropEvent *event)
{
    QString path = draggedNotePath(event->mimeData());
    if (path.isEmpty()) {
        event->ignore();
        setDropHighlighted(false);
        return;
    }

    setDropHighlighted(false);
    event->acceptProposedAction();
    emit noteDropped(path, m_date);
}

QString ReviewTimelineNode::draggedNotePath(const QMimeData *mimeData) const
{
    if (!mimeData)
        return QString();

    if (mimeData->hasUrls()) {
        for (const QUrl &url : mimeData->urls()) {
            QString path = url.toLocalFile();
            QFileInfo info(path);
            if (info.isFile() && info.suffix() == "md")
                return info.absoluteFilePath();
        }
    }

    return QString();
}

QString ReviewTimelineNode::dateLabel() const
{
    return QString::number(m_date.month()) + QStringLiteral("/") + QString::number(m_date.day());
}

void ReviewTimelineNode::setDropHighlighted(bool highlighted)
{
    setProperty("dropHighlighted", highlighted);
    style()->unpolish(this);
    style()->polish(this);
}

void ReviewTimelineNode::setHasReviews(bool hasReviews)
{
    setProperty("hasReviews", hasReviews);
    style()->unpolish(this);
    style()->polish(this);
    m_dot->setProperty("hasReviews", hasReviews);
    m_dot->style()->unpolish(m_dot);
    m_dot->style()->polish(m_dot);
}

ReviewTimelinePane::ReviewTimelinePane(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_content(nullptr)
    , m_timelineLayout(nullptr)
    , m_emptyLabel(nullptr)
    , m_preview(nullptr)
    , m_rememberButton(nullptr)
    , m_forgetButton(nullptr)
    , m_strategyButton(nullptr)
{
    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(24, 22, 24, 22);
    rootLayout->setSpacing(16);

    QWidget *timelinePanel = new QWidget(this);
    QVBoxLayout *timelineLayout = new QVBoxLayout(timelinePanel);
    timelineLayout->setContentsMargins(0, 0, 0, 0);
    timelineLayout->setSpacing(14);

    QLabel *title = new QLabel(tr("复习时间轴"), this);
    title->setObjectName("timelineTitle");
    QLabel *subtitle = new QLabel(tr("拖动左侧笔记到日期列，安排未来 30 天内的复习。"), this);
    subtitle->setObjectName("timelineSubtitle");
    timelineLayout->addWidget(title);
    timelineLayout->addWidget(subtitle);

    m_emptyLabel = new QLabel(tr("暂无复习计划。仍可把左侧笔记拖到下方日期列安排复习。"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setObjectName("timelineEmptyState");
    timelineLayout->addWidget(m_emptyLabel);

    m_scrollArea = new QScrollArea(timelinePanel);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_content = new QWidget(m_scrollArea);
    m_content->setObjectName("timelineContent");
    m_timelineLayout = new QHBoxLayout(m_content);
    m_timelineLayout->setContentsMargins(0, 0, 0, 0);
    m_timelineLayout->setSpacing(0);
    m_scrollArea->setWidget(m_content);

    timelineLayout->addWidget(m_scrollArea, 1);

    QWidget *previewPanel = new QWidget(this);
    QVBoxLayout *previewLayout = new QVBoxLayout(previewPanel);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(10);

    m_preview = new PreviewPane(previewPanel);
    m_preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_preview->setHtml(tr("<div style='color:#8a8a8a;padding:24px;'>点击左侧时间轴上的笔记卡片进行预览。</div>"));

    QWidget *buttonBar = new QWidget(previewPanel);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);
    m_rememberButton = new QPushButton(tr("记住"), buttonBar);
    m_forgetButton = new QPushButton(tr("忘记"), buttonBar);
    m_strategyButton = new QPushButton(tr("调整策略"), buttonBar);
    m_rememberButton->setObjectName("timelineRememberButton");
    m_forgetButton->setObjectName("timelineForgetButton");
    m_strategyButton->setObjectName("timelineStrategyButton");
    buttonLayout->addWidget(m_rememberButton);
    buttonLayout->addWidget(m_forgetButton);
    buttonLayout->addWidget(m_strategyButton);

    previewLayout->addWidget(m_preview, 1);
    previewLayout->addWidget(buttonBar);

    QFrame *divider = new QFrame(this);
    divider->setObjectName("timelinePreviewDivider");
    divider->setFixedWidth(1);
    divider->setFrameShape(QFrame::NoFrame);

    rootLayout->addWidget(timelinePanel, 6);
    rootLayout->addWidget(divider);
    rootLayout->addWidget(previewPanel, 5);

    connect(m_rememberButton, &QPushButton::clicked, this, [this]() {
        if (!m_selectedNotePath.isEmpty())
            emit rememberedRequested(m_selectedItem);
    });
    connect(m_forgetButton, &QPushButton::clicked, this, [this]() {
        if (!m_selectedNotePath.isEmpty())
            emit forgottenRequested(m_selectedItem);
    });
    connect(m_strategyButton, &QPushButton::clicked, this, [this]() {
        if (!m_selectedNotePath.isEmpty())
            emit strategyAdjustRequested(m_selectedItem);
    });
    updateReviewButtons();

    setStyleSheet(
        "QLabel#timelineTitle { font-size: 24px; font-weight: 700; }"
        "QLabel#timelineSubtitle { color: #8a8a8a; }"
        "QLabel#timelineEmptyState { color: #8a8a8a; padding: 4px; }"
        "QWidget#timelineContent { border-bottom: 1px solid rgba(128,128,128,0.22); }"
        "QFrame#timelinePreviewDivider { background: rgba(128,128,128,0.22); }"
        "QWidget#timelineDayColumn { background: transparent; border-radius: 10px; }"
        "QWidget#timelineDayColumn[hasReviews=\"true\"] { background: rgba(128,128,128,0.035); }"
        "QWidget#timelineDayColumn[dropHighlighted=\"true\"] { background: rgba(46,128,242,0.14); }"
        "QLabel#timelineNodeLabel { color: #8a8a8a; font-size: 12px; font-weight: 600; }"
        "QLabel#timelineNodeDot { color: rgba(128,128,128,0.45); font-size: 10px; }"
        "QLabel#timelineNodeDot[hasReviews=\"true\"] { color: rgba(46,128,242,0.65); }"
        "QPushButton#timelineNoteButton { background: rgba(46,128,242,0.12); color: palette(text); border: 1px solid rgba(46,128,242,0.25); border-radius: 8px; padding: 6px 10px; text-align: left; font-weight: 600; font-size: 13px; }"
        "QPushButton#timelineNoteButton:hover { background: rgba(46,128,242,0.22); border-color: #2e80f2; }"
        "QPushButton#timelineRememberButton { background: #34c759; color: #ffffff; border: none; border-radius: 7px; padding: 7px 10px; font-weight: 600; }"
        "QPushButton#timelineForgetButton { background: #ff3b30; color: #ffffff; border: none; border-radius: 7px; padding: 7px 10px; font-weight: 600; }"
        "QPushButton#timelineStrategyButton { background: transparent; color: palette(text); border: 1px solid rgba(128,128,128,0.28); border-radius: 7px; padding: 7px 10px; }"
        "QPushButton:disabled { background: rgba(128,128,128,0.18); color: rgba(128,128,128,0.65); border: none; }"
    );
}

QDate ReviewTimelinePane::startDate() const
{
    return QDate::currentDate();
}

QDate ReviewTimelinePane::endDate() const
{
    return startDate().addDays(30);
}

void ReviewTimelinePane::setReviewPlan(const QList<ReviewPlanItem> &reviews)
{
    m_reviews = reviews;
    rebuildNodes();
}

void ReviewTimelinePane::rebuildNodes()
{
    QMap<QDate, QList<ReviewPlanItem>> grouped = groupedReviews();
    m_emptyLabel->setVisible(m_reviews.isEmpty());

    ensureNodes();
    updateContentHeight(grouped);
    for (ReviewTimelineNode *node : m_nodes)
        node->setReviews(grouped.value(node->date()));
}

void ReviewTimelinePane::updateContentHeight(const QMap<QDate, QList<ReviewPlanItem>> &grouped)
{
    int maxCount = 0;
    for (const QList<ReviewPlanItem> &reviews : grouped)
        maxCount = qMax(maxCount, reviews.size());

    int minHeight = 140 + qMax(0, maxCount) * 52;
    minHeight = qMax(minHeight, 360);
    m_content->setMinimumHeight(minHeight);
}

void ReviewTimelinePane::ensureNodes()
{
    if (!m_nodes.isEmpty() && m_nodesStartDate == startDate())
        return;

    clearNodes();
    m_nodesStartDate = startDate();

    for (QDate date = startDate(); date <= endDate(); date = date.addDays(1)) {
        ReviewTimelineNode *node = new ReviewTimelineNode(date, m_content);
        m_nodes.append(node);
        m_timelineLayout->addWidget(node);
        connect(node, &ReviewTimelineNode::noteDropped, this, [this](const QString &path, const QDate &date) {
            QTimer::singleShot(0, this, [this, path, date]() {
                emit noteDropped(path, date);
            });
        });
        connect(node, &ReviewTimelineNode::noteOpenRequested,
                this, &ReviewTimelinePane::noteOpenRequested);
        connect(node, &ReviewTimelineNode::notePreviewRequested,
                this, &ReviewTimelinePane::notePreviewRequested);
    }
}

void ReviewTimelinePane::showPreview(const ReviewPlanItem &item, const QString &markdown)
{
    m_selectedNotePath = item.noteId;
    m_selectedItem = item;
    m_preview->showHtml(markdown);
    m_preview->verticalScrollBar()->setValue(0);
    updateReviewButtons();
}

void ReviewTimelinePane::clearPreview()
{
    m_selectedNotePath.clear();
    m_selectedItem = ReviewPlanItem();
    m_preview->setHtml(tr("<div style='color:#8a8a8a;padding:24px;'>点击左侧时间轴上的笔记卡片进行预览。</div>"));
    updateReviewButtons();
}

void ReviewTimelinePane::updateReviewButtons()
{
    bool hasSelection = !m_selectedNotePath.isEmpty();
    m_rememberButton->setEnabled(hasSelection);
    m_forgetButton->setEnabled(hasSelection);
    m_strategyButton->setEnabled(hasSelection);
}

void ReviewTimelinePane::clearNodes()
{
    while (QLayoutItem *item = m_timelineLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    m_nodes.clear();
}

QMap<QDate, QList<ReviewPlanItem>> ReviewTimelinePane::groupedReviews() const
{
    QMap<QDate, QList<ReviewPlanItem>> grouped;
    for (const ReviewPlanItem &item : m_reviews)
        grouped[item.reviewTime.date()].append(item);
    return grouped;
}
