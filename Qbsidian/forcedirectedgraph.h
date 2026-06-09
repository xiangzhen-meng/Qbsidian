#ifndef FORCEDIRECTEDGRAPH_H
#define FORCEDIRECTEDGRAPH_H

#include <QString>
#include <QHash>
#include <QList>

struct GraphNode {
    QString id;
    double x = 0.0;
    double y = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double fx = 0.0;
    double fy = 0.0;
};

struct GraphEdge {
    QString sourceId;
    QString targetId;
};

class ForceDirectedGraph
{
public:
    ForceDirectedGraph();
    ~ForceDirectedGraph();

    void addNode(const QString &id);
    void addEdge(const QString &sourceId, const QString &targetId);

    void addHiddenEdge(const QString &sourceId, const QString &targetId);

    void clear();

    void updatePhysicsStep();

    const QHash<QString, GraphNode*>& getNodes() const { return m_nodes; }
    const QList<GraphEdge>& getEdges() const { return m_edges; }

    void setNodePosition(const QString &id, double x, double y);

private:
    QHash<QString, GraphNode*> m_nodes;
    QList<GraphEdge> m_edges;
    QList<GraphEdge> m_hiddenEdges;

    const double m_repulsionK = 55000.0;
    const double m_springK = 0.06;
    const double m_idealLength = 80.0;
    const double m_damping = 0.70;
    const double m_gravityK = 0.015;
};

#endif // FORCEDIRECTEDGRAPH_H
