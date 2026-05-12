#ifndef FILEEXPLORERPANE_H
#define FILEEXPLORERPANE_H

#include <QWidget>

class QTreeView;
class QFileSystemModel;

class FileExplorerPane : public QWidget
{
    Q_OBJECT

public:
    explicit FileExplorerPane(QWidget *parent = nullptr);

    void setRootPath(const QString &path);

signals:
    void fileSelected(const QString &absoluteFilePath);
    void newNoteRequested(const QString &directory, const QString &baseName);
    void newFolderRequested(const QString &directory, const QString &folderName);

private slots:
    void onItemClicked(const QModelIndex &index);
    void onCustomContextMenu(const QPoint &pos);

private:
    QTreeView *m_treeView;
    QFileSystemModel *m_model;
};

#endif // FILEEXPLORERPANE_H
