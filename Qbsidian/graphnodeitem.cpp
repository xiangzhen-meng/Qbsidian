#include "graphnodeitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QFontMetrics>

static const int kCircleRadius = 8;
static const int kTextGap = 4;
static const int kMaxTextWidth = 160;

GraphNodeItem::GraphNodeItem(const QString &id, bool darkMode, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_id(id)
    , m_darkMode(darkMode)
    , m_hovered(false)
    , m_cachedRect(-24, -24, 48, 48)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    setToolTip(m_id);

    m_cachedFont.setPixelSize(11);
    m_cachedFont.setFamily("LXGW WenKai Screen");
}

void GraphNodeItem::setDarkMode(bool dark)
{
    if (m_darkMode == dark)
        return;
    m_darkMode = dark;
    update();
}

QRectF GraphNodeItem::boundingRect() const
{
    QFontMetrics fm(m_cachedFont);
    int textWidth = qMin(fm.horizontalAdvance(m_id), kMaxTextWidth);
    int textHeight = fm.height();

    int padding = 6;
    int halfW = qMax(textWidth / 2 + padding, kCircleRadius + padding);
    int top = -kCircleRadius - padding;
    int bottom = kCircleRadius + kTextGap + textHeight + padding;

    m_cachedRect = QRectF(-halfW, top, halfW * 2, bottom - top);
    return m_cachedRect;
}

void GraphNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QColor nodeColor = m_darkMode ? QColor("#5e81ac") : QColor("#81a1c1");
    QColor textColor = m_darkMode ? QColor("#e5e9f0") : QColor("#2E3440");

    if (m_hovered) {
        painter->setPen(QPen(QColor("#FFD166"), 2));
        painter->setBrush(QColor("#D4A017"));
    } else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(nodeColor);
    }
    painter->drawEllipse(QPointF(0, 0), kCircleRadius, kCircleRadius);

    painter->setPen(textColor);
    painter->setFont(m_cachedFont);

    QFontMetrics fm(m_cachedFont);
    QString label = m_id;
    if (fm.horizontalAdvance(label) > kMaxTextWidth)
        label = fm.elidedText(m_id, Qt::ElideMiddle, kMaxTextWidth);

    int textWidth = fm.horizontalAdvance(label);
    int textHeight = fm.height();
    QRectF textRect(-textWidth / 2.0, kCircleRadius + kTextGap, textWidth, textHeight + 4);
    painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, label);
}

QVariant GraphNodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged)
        emit moved(m_id, value.toPointF());
    return QGraphicsObject::itemChange(change, value);
}

void GraphNodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    emit clicked(m_id);
}

void GraphNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_hovered = true;
    update();
}

void GraphNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_hovered = false;
    update();
}
