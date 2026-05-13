#include "fileexplorerpane.h"
#include "autohidescrollareafilter.h"
#include <QTreeView>
#include <QFileSystemModel>
#include <QMenu>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QFileInfo>

FileExplorerPane::FileExplorerPane(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *toolbar = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(4, 4, 4, 4);
    toolbarLayout->setSpacing(4);

    QPushButton *btnNewNote = new QPushButton(toolbar);
    QPushButton *btnNewFolder = new QPushButton(toolbar);
    btnNewNote->setFixedSize(28, 28);
    btnNewFolder->setFixedSize(28, 28);
    btnNewNote->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    btnNewFolder->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    btnNewNote->setIconSize(QSize(16, 16));
    btnNewFolder->setIconSize(QSize(16, 16));
    btnNewNote->setToolTip(tr("新建笔记"));
    btnNewFolder->setToolTip(tr("新建文件夹"));
    btnNewNote->setFlat(true);
    btnNewFolder->setFlat(true);
    btnNewNote->setStyleSheet("background: transparent; border: none;");
    btnNewFolder->setStyleSheet("background: transparent; border: none;");
    toolbarLayout->addWidget(btnNewNote);
    toolbarLayout->addWidget(btnNewFolder);
    toolbarLayout->addStretch();
    layout->addWidget(toolbar);

    m_treeView = new QTreeView(this);
    m_model = new QFileSystemModel(this);

    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_model->setNameFilters({"*.md"});
    m_model->setNameFilterDisables(false);

    m_treeView->setModel(m_model);
    m_treeView->setHeaderHidden(true);
    m_treeView->setIndentation(12);
    for (int i = 1; i < m_model->columnCount(); ++i)
        m_treeView->hideColumn(i);

    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::clicked,
            this, &FileExplorerPane::onItemClicked);
    connect(m_treeView, &QWidget::customContextMenuRequested,
            this, &FileExplorerPane::onCustomContextMenu);

    connect(btnNewNote, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        QString name = QInputDialog::getText(this, tr("新建笔记"),
                                             tr("笔记名称（不含扩展名）:"),
                                             QLineEdit::Normal, QString(), &ok);
        if (ok && !name.isEmpty())
            emit newNoteRequested(rootPath(), name);
    });
    connect(btnNewFolder, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        QString name = QInputDialog::getText(this, tr("新建文件夹"),
                                             tr("文件夹名称:"),
                                             QLineEdit::Normal, QString(), &ok);
        if (ok && !name.isEmpty())
            emit newFolderRequested(rootPath(), name);
    });

    new AutoHideScrollAreaFilter(m_treeView, this);
}

QString FileExplorerPane::rootPath() const
{
    return m_model->rootPath();
}

void FileExplorerPane::setRootPath(const QString &path)
{
    QModelIndex rootIndex = m_model->setRootPath(path);
    m_treeView->setRootIndex(rootIndex);
}

void FileExplorerPane::onItemClicked(const QModelIndex &index)
{
    QFileInfo info = m_model->fileInfo(index);
    if (info.isFile() && info.suffix() == "md")
        emit fileSelected(info.absoluteFilePath());
}

void FileExplorerPane::onCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = m_treeView->indexAt(pos);
    QString dirPath;
    if (index.isValid()) {
        QFileInfo info = m_model->fileInfo(index);
        dirPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    } else {
        dirPath = m_model->rootPath();
    }

    QMenu menu(m_treeView);
    QAction *newNoteAction = menu.addAction(tr("新建笔记"));
    QAction *newFolderAction = menu.addAction(tr("新建文件夹"));

    QAction *chosen = menu.exec(m_treeView->viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == newNoteAction) {
        bool ok = false;
        QString name = QInputDialog::getText(this, tr("新建笔记"),
                                             tr("笔记名称（不含扩展名）:"),
                                             QLineEdit::Normal, QString(), &ok);
        if (ok && !name.isEmpty())
            emit newNoteRequested(dirPath, name);
    } else if (chosen == newFolderAction) {
        bool ok = false;
        QString name = QInputDialog::getText(this, tr("新建文件夹"),
                                             tr("文件夹名称:"),
                                             QLineEdit::Normal, QString(), &ok);
        if (ok && !name.isEmpty())
            emit newFolderRequested(dirPath, name);
    }
}
