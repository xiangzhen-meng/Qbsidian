#ifndef GRAPHPANE_H
#define GRAPHPANE_H

#include <QWidget>
#include <QHash>
#include <QList>
#include <QString>

class GraphNodeItem;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsLineItem;
class QTimer;
class ForceDirectedGraph;

struct GraphEdgeItem {
    QString sourceId;
    QString targetId;
    QGraphicsLineItem *line;
};

class GraphPane : public QWidget
{
    Q_OBJECT

public:
    explicit GraphPane(QWidget *parent = nullptr);
    ~GraphPane();

    void loadVault(const QString &vaultPath);
    void setDarkMode(bool dark);

signals:
    void noteOpenRequested(const QString &filePath);

private slots:
    void updateFrame();
    void onNodeMoved(const QString &id, const QPointF &pos);
    void onNodeClicked(const QString &id);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void rebuildGraphData(const QString &vaultPath);
    void rebuildSceneItems();
    void syncSceneFromGraph();
    void clearGraph();

    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    ForceDirectedGraph *m_graph;
    QTimer *m_physicsTimer;
    bool m_darkMode;
    QString m_vaultPath;
    double m_currentScale;

    QHash<QString, GraphNodeItem *> m_nodeItems;
    QHash<QString, QString> m_nodeIdToPath;
    QList<GraphEdgeItem> m_edgeItems;
};

#endif // GRAPHPANE_H
