#ifndef FORCEDIRECTEDGRAPH_H
#define FORCEDIRECTEDGRAPH_H

#include <QString>
#include <QHash>
#include <QList>

// 数据节点（笔记）
struct GraphNode {
    QString id;       // 笔记唯一标识（文件名）
    double x = 0.0;   // 当前物理坐标 X
    double y = 0.0;   // 当前物理坐标 Y
    double vx = 0.0;  // 速度 X (Velocity)
    double vy = 0.0;  // 速度 Y
    double fx = 0.0;  // 瞬间受力 X (Force)
    double fy = 0.0;  // 瞬间受力 Y
};

// 数据边（引用关系）
struct GraphEdge {
    QString sourceId;
    QString targetId;
};

// 物理引擎
class ForceDirectedGraph
{
public:
    ForceDirectedGraph();
    ~ForceDirectedGraph();

    // 后端输入接口
    void addNode(const QString &id);
    void addEdge(const QString &sourceId, const QString &targetId);
    void clear();

    // 计算单个物理帧的迭代
    // 每次被调用，都会让图谱向平衡状态演进一帧
    void updatePhysicsStep();

    // 后端输出接口
    const QHash<QString, GraphNode*>& getNodes() const { return m_nodes; }
    const QList<GraphEdge>& getEdges() const { return m_edges; }

    // 当用户在UI上拖拽某个节点时，UI调用此接口更新其物理位置
    void setNodePosition(const QString &id, double x, double y);

private:
    QHash<QString, GraphNode*> m_nodes;
    QList<GraphEdge> m_edges;

    // 物理常数（可控制图谱聚集度）
    const double m_repulsionK = 45000.0; // 斥力常数 (值越大，节点散得越开)
    const double m_springK = 0.05;       // 弹簧引力常数 (值越大，连线拉得越紧)
    const double m_idealLength = 80.0;   // 弹簧理想长度 (相连节点间的舒适距离)
    const double m_damping = 0.75;       // 摩擦力/阻尼 (值在0-1之间，越小系统越快静止，防止永无止境地颤动)
    const double m_gravityK = 0.01;      // 重力常数 (防止孤立节点飞出屏幕外，给所有节点一个向中心(0,0)拉的微弱力)
};

#endif // FORCEDIRECTEDGRAPH_H
