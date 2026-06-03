#ifndef GRAPHNODEITEM_H
#define GRAPHNODEITEM_H

#include <QGraphicsObject>
#include <QString>
#include <QFont>

class GraphNodeItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit GraphNodeItem(const QString &id, bool darkMode = false, QGraphicsItem *parent = nullptr);

    QString id() const { return m_id; }
    void setDarkMode(bool dark);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void moved(const QString &id, const QPointF &pos);
    void clicked(const QString &id);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString m_id;
    bool m_darkMode;
    mutable QRectF m_cachedRect;
    mutable QFont m_cachedFont;
};

#endif // GRAPHNODEITEM_H
