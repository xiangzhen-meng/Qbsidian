#include "forcedirectedgraph.h"
#include <QtMath>
#include <QRandomGenerator>

ForceDirectedGraph::ForceDirectedGraph() {}

ForceDirectedGraph::~ForceDirectedGraph() { clear(); }

void ForceDirectedGraph::clear()
{
    qDeleteAll(m_nodes);
    m_nodes.clear();
    m_edges.clear();
    m_hiddenEdges.clear();
}

void ForceDirectedGraph::addNode(const QString &id)
{
    if (m_nodes.contains(id)) return;

    GraphNode *node = new GraphNode();
    node->id = id;
    node->x = QRandomGenerator::global()->bounded(200.0) - 100.0;
    node->y = QRandomGenerator::global()->bounded(200.0) - 100.0;

    m_nodes.insert(id, node);
}

void ForceDirectedGraph::addEdge(const QString &sourceId, const QString &targetId)
{
    addNode(sourceId);
    addNode(targetId);
    m_edges.append(GraphEdge{sourceId, targetId});
}


void ForceDirectedGraph::addHiddenEdge(const QString &sourceId, const QString &targetId)
{
    addNode(sourceId);
    addNode(targetId);
    m_hiddenEdges.append(GraphEdge{sourceId, targetId});
}

void ForceDirectedGraph::setNodePosition(const QString &id, double x, double y)
{
    if (m_nodes.contains(id)) {
        m_nodes[id]->x = x;
        m_nodes[id]->y = y;
        m_nodes[id]->vx = 0.0;
        m_nodes[id]->vy = 0.0;
    }
}

void ForceDirectedGraph::updatePhysicsStep()
{
    const double epsilon = 0.0001;
    for (GraphNode *node : m_nodes) {
        node->fx = 0.0;
        node->fy = 0.0;
    }

    QList<GraphNode*> nodeList = m_nodes.values();
    for (int i = 0; i < nodeList.size(); ++i) {
        for (int j = i + 1; j < nodeList.size(); ++j) {
            GraphNode *nA = nodeList[i];
            GraphNode *nB = nodeList[j];

            double dx = nB->x - nA->x;
            double dy = nB->y - nA->y;
            double dist = qSqrt(dx * dx + dy * dy) + epsilon;

            double fRepel = m_repulsionK / (dist * dist);
            double fx = (dx / dist) * fRepel;
            double fy = (dy / dist) * fRepel;

            nA->fx -= fx;
            nA->fy -= fy;
            nB->fx += fx;
            nB->fy += fy;
        }
    }

    for (const GraphEdge &edge : m_edges) {
        GraphNode *nA = m_nodes[edge.sourceId];
        GraphNode *nB = m_nodes[edge.targetId];

        double dx = nB->x - nA->x;
        double dy = nB->y - nA->y;
        double dist = qSqrt(dx * dx + dy * dy) + epsilon;

        double fSpring = m_springK * (dist - m_idealLength*2);
        double fx = (dx / dist) * fSpring;
        double fy = (dy / dist) * fSpring;

        nA->fx += fx;
        nA->fy += fy;
        nB->fx -= fx;
        nB->fy -= fy;
    }

    for (const GraphEdge &edge : m_hiddenEdges) {
        GraphNode *nA = m_nodes[edge.sourceId];
        GraphNode *nB = m_nodes[edge.targetId];

        double dx = nB->x - nA->x;
        double dy = nB->y - nA->y;
        double dist = qSqrt(dx * dx + dy * dy) + epsilon;

        double fSpring = (m_springK * 1.5) * (dist - (m_idealLength * 0.4));
        double fx = (dx / dist) * fSpring;
        double fy = (dy / dist) * fSpring;

        nA->fx += fx;
        nA->fy += fy;
        nB->fx -= fx;
        nB->fy -= fy;
    }

    for (GraphNode *node : m_nodes){
        node->fx -= m_gravityK * node->x;
        node->fy -= m_gravityK * node->y;
    }

    for (GraphNode *node : m_nodes) {
        node->vx = (node->vx + node->fx) * m_damping;
        node->vy = (node->vy + node->fy) * m_damping;

        node->x += node->vx;
        node->y += node->vy;
    }
}
