#include "autohidescrollareafilter.h"
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>

AutoHideScrollAreaFilter::AutoHideScrollAreaFilter(QAbstractScrollArea *area, QObject *parent)
    : QObject(parent)
    , m_area(area)
    , m_hideTimer(new QTimer(this))
    , m_scrollBarActive(false)
{
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(1200);
    connect(m_hideTimer, &QTimer::timeout, this, [this]() {
        m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    });

    auto onScrollBarPressed = [this]() {
        m_scrollBarActive = true;
        m_hideTimer->stop();
        m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    };
    auto onScrollBarReleased = [this]() {
        m_scrollBarActive = false;
        m_hideTimer->start();
    };

    connect(m_area->verticalScrollBar(), &QScrollBar::sliderPressed, this, onScrollBarPressed);
    connect(m_area->verticalScrollBar(), &QScrollBar::sliderReleased, this, onScrollBarReleased);
    connect(m_area->horizontalScrollBar(), &QScrollBar::sliderPressed, this, onScrollBarPressed);
    connect(m_area->horizontalScrollBar(), &QScrollBar::sliderReleased, this, onScrollBarReleased);

    m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_area->viewport()->installEventFilter(this);
}

bool AutoHideScrollAreaFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_area->viewport())
        return QObject::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::MouseMove: {
        if (m_scrollBarActive)
            break;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        bool nearEdge = (me->pos().x() >= m_area->viewport()->width() - 20);
        if (nearEdge) {
            m_hideTimer->stop();
            m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else {
            m_hideTimer->start();
        }
        break;
    }
    case QEvent::Wheel:
        m_hideTimer->stop();
        m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        break;
    case QEvent::Leave:
        if (!m_scrollBarActive)
            m_hideTimer->start();
        break;
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}
