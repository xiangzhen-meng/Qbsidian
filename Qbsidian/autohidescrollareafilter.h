#ifndef AUTOHIDESCROLLAREAFILTER_H
#define AUTOHIDESCROLLAREAFILTER_H

#include <QObject>
#include <QAbstractScrollArea>
#include <QTimer>

class AutoHideScrollAreaFilter : public QObject
{
    Q_OBJECT

public:
    explicit AutoHideScrollAreaFilter(QAbstractScrollArea *area, QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QAbstractScrollArea *m_area;
    QTimer *m_hideTimer;
    bool m_scrollBarActive;
};

#endif // AUTOHIDESCROLLAREAFILTER_H
