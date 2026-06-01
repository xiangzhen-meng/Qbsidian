#ifndef FILEEXPLORERPANE_H
#define FILEEXPLORERPANE_H

#include <QWidget>
#include <QFileIconProvider>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

class QTreeView;
class QFileSystemModel;
class QLineEdit;
class QListWidget;
class QStackedWidget;
class QListWidgetItem;
class NoteManager;
class QPushButton;

class QbsidianIconProvider : public QFileIconProvider
{
public:
    QIcon icon(const QFileInfo &info) const override;
};

class DirFirstSortProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

class FileTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    FileTreeDelegate(QTreeView *treeView, DirFirstSortProxy *sortProxy, QFileSystemModel *model, QObject *parent = nullptr);

    void setDarkMode(bool dark);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int depthForIndex(const QModelIndex &index) const;
    bool isDirectory(const QModelIndex &index) const;
    bool isExpandedDirectory(const QModelIndex &index) const;
    QRect iconRectForIndex(const QModelIndex &index, const QRect &itemRect, const QStyleOptionViewItem &baseOption) const;

    QTreeView *m_treeView;
    DirFirstSortProxy *m_sortProxy;
    QFileSystemModel *m_model;
    bool m_darkMode;
};

class FileExplorerPane : public QWidget
{
    Q_OBJECT

public:
    explicit FileExplorerPane(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    void setVaultPath(const QString &path);
    void setNoteManager(NoteManager *manager);
    QString rootPath() const;
    void setReviewButtonChecked(bool checked);
    void setDarkMode(bool dark);

signals:
    void fileSelected(const QString &absoluteFilePath);
    void newNoteRequested(const QString &directory, const QString &baseName);
    void newFolderRequested(const QString &directory, const QString &folderName);
    void deleteRequested(const QString &absolutePath);
    void searchResultClicked(const QString &filePath, int lineNumber);
    void reviewTimelineRequested();

private slots:
    void onItemClicked(const QModelIndex &index);
    void onCustomContextMenu(const QPoint &pos);
    void onSearchReturnPressed();
    void onSearchResultClicked(QListWidgetItem *item);

private:
    QString currentTargetDirectory() const;

    QLineEdit *m_searchBox;
    QStackedWidget *m_stack;
    QPushButton *m_filesButton;
    QPushButton *m_searchButton;
    QPushButton *m_reviewButton;
    QTreeView *m_treeView;
    QListWidget *m_searchResults;
    QFileSystemModel *m_model;
    DirFirstSortProxy *m_sortProxy;
    QbsidianIconProvider *m_iconProvider;
    FileTreeDelegate *m_treeDelegate;
    NoteManager *m_noteManager;
    QString m_vaultPath;
};

#endif // FILEEXPLORERPANE_H
