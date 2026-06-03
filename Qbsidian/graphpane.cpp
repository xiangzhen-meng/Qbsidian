#include "graphpane.h"
#include "graphnodeitem.h"
#include "forcedirectedgraph.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QVBoxLayout>
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QPen>
#include <QWheelEvent>
#include <cmath>

static const double kZoomFactor = 1.15;
static const double kMinScale = 0.2;
static const double kMaxScale = 4.0;

GraphPane::GraphPane(QWidget *parent)
    : QWidget(parent)
    , m_scene(new QGraphicsScene(this))
    , m_view(new QGraphicsView(m_scene, this))
    , m_graph(new ForceDirectedGraph)
    , m_physicsTimer(new QTimer(this))
    , m_darkMode(false)
    , m_currentScale(1.0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->viewport()->installEventFilter(this);

    m_physicsTimer->setInterval(16);
    connect(m_physicsTimer, &QTimer::timeout, this, &GraphPane::updateFrame);
}

GraphPane::~GraphPane()
{
    clearGraph();
    delete m_graph;
}

bool GraphPane::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_view->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        double notches = wheelEvent->angleDelta().y() / 120.0;
        if (qFuzzyIsNull(notches))
            return true;

        double factor = std::pow(kZoomFactor, notches);
        double nextScale = m_currentScale * factor;

        if (nextScale < kMinScale || nextScale > kMaxScale)
            return true;

        m_currentScale = nextScale;
        m_view->scale(factor, factor);
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void GraphPane::loadVault(const QString &vaultPath)
{
    m_vaultPath = vaultPath;
    clearGraph();

    rebuildGraphData(vaultPath);
    rebuildSceneItems();

    m_view->resetTransform();
    m_currentScale = 1.0;
    m_physicsTimer->start();
}

void GraphPane::setDarkMode(bool dark)
{
    m_darkMode = dark;

    QColor bgColor = dark ? QColor("#2E3440") : QColor("#ffffff");
    m_view->setBackgroundBrush(QBrush(bgColor));

    QColor edgeColor = dark ? QColor("#5e81ac") : QColor("#b0b0b0");
    QPen pen(edgeColor, 1.5, Qt::SolidLine, Qt::RoundCap);
    for (const GraphEdgeItem &edgeItem : m_edgeItems) {
        edgeItem.line->setPen(pen);
    }

    for (GraphNodeItem *nodeItem : m_nodeItems) {
        nodeItem->setDarkMode(dark);
    }
}

void GraphPane::updateFrame()
{
    m_graph->updatePhysicsStep();
    syncSceneFromGraph();
    m_scene->update();
}

void GraphPane::onNodeMoved(const QString &id, const QPointF &pos)
{
    m_graph->setNodePosition(id, pos.x(), pos.y());
}

void GraphPane::onNodeClicked(const QString &id)
{
    QString path = m_nodeIdToPath.value(id);
    if (!path.isEmpty())
        emit noteOpenRequested(path);
}

void GraphPane::rebuildGraphData(const QString &vaultPath)
{
    m_nodeIdToPath.clear();

    QDirIterator it(vaultPath, QStringList() << "*.md", QDir::Files, QDirIterator::Subdirectories);

    QMap<QString, QStringList> adjacencyMap;
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        QString nodeId = fileInfo.completeBaseName();
        QString absPath = fileInfo.absoluteFilePath();
        m_graph->addNode(nodeId);
        m_nodeIdToPath.insert(nodeId, absPath);

        QFile file(absPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QTextStream stream(&file);
        QString content = stream.readAll();
        file.close();

        QRegularExpression linkRegex(R"(\[\[([^\]\|#]+)(?:[|#][^\]]*)?\]\])");
        QRegularExpressionMatchIterator matchIt = linkRegex.globalMatch(content);
        while (matchIt.hasNext()) {
            QRegularExpressionMatch match = matchIt.next();
            QString targetName = match.captured(1).trimmed();
            if (targetName.isEmpty() || targetName == nodeId)
                continue;
            adjacencyMap[nodeId].append(targetName);
        }
    }

    for (auto iter = adjacencyMap.constBegin(); iter != adjacencyMap.constEnd(); ++iter) {
        for (const QString &targetId : iter.value()) {
            m_graph->addEdge(iter.key(), targetId);
        }
    }
}

void GraphPane::rebuildSceneItems()
{
    m_scene->clear();

    const QHash<QString, GraphNode *> &nodes = m_graph->getNodes();
    for (auto iter = nodes.constBegin(); iter != nodes.constEnd(); ++iter) {
        GraphNodeItem *item = new GraphNodeItem(iter.key(), m_darkMode);
        item->setPos(iter.value()->x, iter.value()->y);
        m_scene->addItem(item);
        m_nodeItems.insert(iter.key(), item);

        connect(item, &GraphNodeItem::moved, this, &GraphPane::onNodeMoved);
        connect(item, &GraphNodeItem::clicked, this, &GraphPane::onNodeClicked);
    }

    const QList<GraphEdge> &edges = m_graph->getEdges();
    for (const GraphEdge &edge : edges) {
        GraphNodeItem *sourceItem = m_nodeItems.value(edge.sourceId);
        GraphNodeItem *targetItem = m_nodeItems.value(edge.targetId);
        if (!sourceItem || !targetItem)
            continue;

        QGraphicsLineItem *lineItem = m_scene->addLine(
            QLineF(sourceItem->pos(), targetItem->pos()),
            QPen(m_darkMode ? QColor("#5e81ac") : QColor("#b0b0b0"), 1.5, Qt::SolidLine, Qt::RoundCap));
        lineItem->setZValue(-1);

        GraphEdgeItem edgeItem;
        edgeItem.sourceId = edge.sourceId;
        edgeItem.targetId = edge.targetId;
        edgeItem.line = lineItem;
        m_edgeItems.append(edgeItem);
    }
}

void GraphPane::syncSceneFromGraph()
{
    const QHash<QString, GraphNode *> &nodes = m_graph->getNodes();
    for (auto iter = nodes.constBegin(); iter != nodes.constEnd(); ++iter) {
        GraphNodeItem *item = m_nodeItems.value(iter.key());
        if (item && !item->isSelected()) {
            item->blockSignals(true);
            item->setPos(iter.value()->x, iter.value()->y);
            item->blockSignals(false);
        }
    }

    for (const GraphEdgeItem &edgeItem : m_edgeItems) {
        GraphNodeItem *sourceItem = m_nodeItems.value(edgeItem.sourceId);
        GraphNodeItem *targetItem = m_nodeItems.value(edgeItem.targetId);
        if (sourceItem && targetItem) {
            edgeItem.line->setLine(QLineF(sourceItem->pos(), targetItem->pos()));
        }
    }
}

void GraphPane::clearGraph()
{
    m_physicsTimer->stop();
    m_scene->clear();
    m_nodeItems.clear();
    m_nodeIdToPath.clear();
    m_edgeItems.clear();
    m_graph->clear();
}
