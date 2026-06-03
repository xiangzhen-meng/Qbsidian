#ifndef REVIEWTIMELINEPANE_H
#define REVIEWTIMELINEPANE_H

#include <QDate>
#include <QMap>
#include <QPushButton>
#include <QWidget>
#include "ReviewManager.h"

class QFrame;
class QHBoxLayout;
class QLabel;
class QMouseEvent;
class QMimeData;
class PreviewPane;
class QScrollArea;
class QVBoxLayout;

class ReviewNoteButton : public QPushButton
{
    Q_OBJECT

public:
    ReviewNoteButton(const ReviewPlanItem &item, QWidget *parent = nullptr);

signals:
    void doubleClicked(const ReviewPlanItem &item);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    ReviewPlanItem m_item;
};

class ReviewTimelineNode : public QWidget
{
    Q_OBJECT

public:
    ReviewTimelineNode(const QDate &date, QWidget *parent = nullptr);

    QDate date() const;
    void setReviews(const QList<ReviewPlanItem> &reviews);
    int reviewCount() const;

signals:
    void noteDropped(const QString &absolutePath, const QDate &date);
    void noteOpenRequested(const QString &absolutePath);
    void notePreviewRequested(const ReviewPlanItem &item);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QString draggedNotePath(const QMimeData *mimeData) const;
    QString dateLabel() const;
    void setDropHighlighted(bool highlighted);
    void setHasReviews(bool hasReviews);

    QDate m_date;
    QVBoxLayout *m_rootLayout;
    QVBoxLayout *m_notesLayout;
    QLabel *m_label;
    QLabel *m_dot;
    int m_reviewCount;
};

class ReviewTimelinePane : public QWidget
{
    Q_OBJECT

public:
    explicit ReviewTimelinePane(QWidget *parent = nullptr);

    QDate startDate() const;
    QDate endDate() const;
    void setReviewPlan(const QList<ReviewPlanItem> &reviews);

signals:
    void noteDropped(const QString &absolutePath, const QDate &date);
    void noteOpenRequested(const QString &absolutePath);
    void notePreviewRequested(const ReviewPlanItem &item);
    void rememberedRequested(const ReviewPlanItem &item);
    void forgottenRequested(const ReviewPlanItem &item);
    void strategyAdjustRequested(const ReviewPlanItem &item);

public slots:
    void showPreview(const ReviewPlanItem &item, const QString &markdown);
    void clearPreview();

private:
    void rebuildNodes();
    void clearNodes();
    void ensureNodes();
    void updateContentHeight(const QMap<QDate, QList<ReviewPlanItem>> &grouped);
    QMap<QDate, QList<ReviewPlanItem>> groupedReviews() const;
    void updateReviewButtons();

    QList<ReviewPlanItem> m_reviews;
    QList<ReviewTimelineNode *> m_nodes;
    QDate m_nodesStartDate;
    QScrollArea *m_scrollArea;
    QWidget *m_content;
    QHBoxLayout *m_timelineLayout;
    QLabel *m_emptyLabel;
    PreviewPane *m_preview;
    QPushButton *m_rememberButton;
    QPushButton *m_forgetButton;
    QPushButton *m_strategyButton;
    QString m_selectedNotePath;
    ReviewPlanItem m_selectedItem;
};

#endif // REVIEWTIMELINEPANE_H
