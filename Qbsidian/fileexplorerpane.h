#ifndef FILEEXPLORERPANE_H
#define FILEEXPLORERPANE_H

#include <QWidget>
#include <QFileIconProvider>
#include <QSortFilterProxyModel>

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
    NoteManager *m_noteManager;
    QString m_vaultPath;
};

#endif // FILEEXPLORERPANE_H
