#ifndef REVIEWTIMELINEPANE_H
#define REVIEWTIMELINEPANE_H

#include <QDate>
#include <QMap>
#include <QWidget>
#include "reviewentity.h"

class QFrame;
class QHBoxLayout;
class QLabel;
class QMimeData;
class PreviewPane;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

class ReviewTimelineNode : public QWidget
{
    Q_OBJECT

public:
    ReviewTimelineNode(const QDate &date, QWidget *parent = nullptr);

    QDate date() const;
    void setReviews(const QList<ReviewEntity> &reviews);
    int reviewCount() const;

signals:
    void noteDropped(const QString &absolutePath, const QDate &date);
    void noteOpenRequested(const QString &absolutePath);
    void notePreviewRequested(const QString &absolutePath);

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
    void setReviewPlan(const QList<ReviewEntity> &reviews);

signals:
    void noteDropped(const QString &absolutePath, const QDate &date);
    void noteOpenRequested(const QString &absolutePath);
    void notePreviewRequested(const QString &absolutePath);
    void familiarRequested(const QString &absolutePath);
    void forgetRequested(const QString &absolutePath);

public slots:
    void showPreview(const QString &absolutePath, const QString &title, const QString &markdown);
    void clearPreview();

private:
    void rebuildNodes();
    void clearNodes();
    void ensureNodes();
    void updateContentHeight(const QMap<QDate, QList<ReviewEntity>> &grouped);
    QMap<QDate, QList<ReviewEntity>> groupedReviews() const;
    void updateReviewButtons();

    QList<ReviewEntity> m_reviews;
    QList<ReviewTimelineNode *> m_nodes;
    QDate m_nodesStartDate;
    QScrollArea *m_scrollArea;
    QWidget *m_content;
    QHBoxLayout *m_timelineLayout;
    QLabel *m_emptyLabel;
    PreviewPane *m_preview;
    QPushButton *m_openButton;
    QPushButton *m_familiarButton;
    QPushButton *m_forgetButton;
    QString m_selectedNotePath;
};

#endif // REVIEWTIMELINEPANE_H
