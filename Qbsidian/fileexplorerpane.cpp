#include "fileexplorerpane.h"
#include "autohidescrollareafilter.h"
#include "notemanager.h"
#include <QTreeView>
#include <QFileSystemModel>
#include <QMenu>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QStackedWidget>
#include <QRegularExpression>
#include <QLabel>
#include <QFrame>
#include <QMimeData>
#include <QUrl>
#include <QAbstractItemView>
#include <QPainter>
#include <QApplication>
#include <QStyleOptionViewItem>

QIcon QbsidianIconProvider::icon(const QFileInfo &info) const
{
    if (info.isDir())
        return QIcon(":/icons/folder.svg");
    if (info.suffix() == "md")
        return QIcon(":/icons/markdown.svg");
    return QFileIconProvider::icon(info);
}

bool DirFirstSortProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QFileSystemModel *fsm = qobject_cast<QFileSystemModel *>(sourceModel());
    if (!fsm)
        return QSortFilterProxyModel::lessThan(left, right);

    QFileInfo leftInfo = fsm->fileInfo(left);
    QFileInfo rightInfo = fsm->fileInfo(right);

    if (leftInfo.isDir() != rightInfo.isDir())
        return leftInfo.isDir();

    return leftInfo.fileName().compare(rightInfo.fileName(), Qt::CaseInsensitive) < 0;
}

Qt::ItemFlags DirFirstSortProxy::flags(const QModelIndex &index) const
{
    Qt::ItemFlags itemFlags = QSortFilterProxyModel::flags(index);
    QFileSystemModel *fsm = qobject_cast<QFileSystemModel *>(sourceModel());
    if (!fsm || !index.isValid())
        return itemFlags;

    QFileInfo info = fsm->fileInfo(mapToSource(index));
    if (info.isFile() && info.suffix() == "md")
        itemFlags |= Qt::ItemIsDragEnabled;
    return itemFlags;
}

QStringList DirFirstSortProxy::mimeTypes() const
{
    return {QStringLiteral("text/uri-list")};
}

QMimeData *DirFirstSortProxy::mimeData(const QModelIndexList &indexes) const
{
    QFileSystemModel *fsm = qobject_cast<QFileSystemModel *>(sourceModel());
    QMimeData *mimeData = new QMimeData;
    if (!fsm)
        return mimeData;

    QList<QUrl> urls;
    for (const QModelIndex &index : indexes) {
        if (!index.isValid() || index.column() != 0)
            continue;

        QFileInfo info = fsm->fileInfo(mapToSource(index));
        if (info.isFile() && info.suffix() == "md")
            urls.append(QUrl::fromLocalFile(info.absoluteFilePath()));
    }
    mimeData->setUrls(urls);
    return mimeData;
}

FileTreeDelegate::FileTreeDelegate(QTreeView *treeView, DirFirstSortProxy *sortProxy, QFileSystemModel *model, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_treeView(treeView)
    , m_sortProxy(sortProxy)
    , m_model(model)
    , m_darkMode(false)
{
}

void FileTreeDelegate::setDarkMode(bool dark)
{
    m_darkMode = dark;
}

void FileTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (!m_treeView || !m_sortProxy || !m_model || !index.isValid())
        return;

    bool expandedDir = isExpandedDirectory(index);
    if (!expandedDir && !index.parent().isValid())
        return;

    painter->save();
    QColor lineColor = m_darkMode ? QColor(255, 255, 255, 22) : QColor(0, 0, 0, 18);
    painter->setPen(QPen(lineColor, 1));

    if (expandedDir) {
        QRect iconRect = iconRectForIndex(index, option.rect, option);
        int x = iconRect.center().x();
        painter->drawLine(x, iconRect.bottom() + 2, x, option.rect.bottom());
    }

    for (QModelIndex ancestor = index.parent(); ancestor.isValid(); ancestor = ancestor.parent()) {
        if (!isExpandedDirectory(ancestor))
            continue;

        QRect ancestorRect = m_treeView->visualRect(ancestor);
        QRect ancestorIconRect = iconRectForIndex(ancestor, ancestorRect, option);
        int x = ancestorIconRect.center().x();
        painter->drawLine(x, option.rect.top(), x, option.rect.bottom());
    }
    painter->restore();
}

int FileTreeDelegate::depthForIndex(const QModelIndex &index) const
{
    int depth = 0;
    QModelIndex parentIndex = index.parent();
    while (parentIndex.isValid()) {
        ++depth;
        parentIndex = parentIndex.parent();
    }
    return depth;
}

bool FileTreeDelegate::isDirectory(const QModelIndex &index) const
{
    if (!index.isValid() || !m_sortProxy || !m_model)
        return false;

    QModelIndex sourceIndex = m_sortProxy->mapToSource(index);
    return m_model->fileInfo(sourceIndex).isDir();
}

bool FileTreeDelegate::isExpandedDirectory(const QModelIndex &index) const
{
    return isDirectory(index) && m_treeView && m_treeView->isExpanded(index);
}

QRect FileTreeDelegate::iconRectForIndex(const QModelIndex &index, const QRect &itemRect, const QStyleOptionViewItem &baseOption) const
{
    QStyleOptionViewItem itemOption(baseOption);
    initStyleOption(&itemOption, index);
    itemOption.rect = itemRect;

    QStyle *style = m_treeView ? m_treeView->style() : QApplication::style();
    return style->subElementRect(QStyle::SE_ItemViewItemDecoration, &itemOption, m_treeView);
}

FileExplorerPane::FileExplorerPane(QWidget *parent)
    : QWidget(parent)
    , m_searchBox(nullptr)
    , m_stack(nullptr)
    , m_filesButton(nullptr)
    , m_searchButton(nullptr)
    , m_reviewButton(nullptr)
    , m_treeView(nullptr)
    , m_searchResults(nullptr)
    , m_model(nullptr)
    , m_sortProxy(nullptr)
    , m_iconProvider(nullptr)
    , m_treeDelegate(nullptr)
    , m_noteManager(nullptr)
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QWidget *navBar = new QWidget(this);
    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(6, 6, 6, 4);
    navLayout->setSpacing(6);

    m_filesButton = new QPushButton(tr("文件"), navBar);
    m_searchButton = new QPushButton(tr("搜索"), navBar);
    m_reviewButton = new QPushButton(tr("复习"), navBar);
    m_filesButton->setCheckable(true);
    m_searchButton->setCheckable(true);
    m_reviewButton->setCheckable(true);
    m_filesButton->setChecked(true);
    m_filesButton->setFixedSize(52, 24);
    m_searchButton->setFixedSize(52, 24);
    m_reviewButton->setFixedSize(52, 24);
    QString navButtonStyle =
        "QPushButton { padding: 2px 6px; font-size: 12px; font-weight: 500; border-radius: 5px; }"
        "QPushButton:checked { background-color: #2e80f2; color: #ffffff; }";
    m_filesButton->setStyleSheet(navButtonStyle);
    m_searchButton->setStyleSheet(navButtonStyle);
    m_reviewButton->setStyleSheet(navButtonStyle);
    m_filesButton->setToolTip(tr("文件树"));
    m_searchButton->setToolTip(tr("搜索"));
    m_reviewButton->setToolTip(tr("复习"));
    navLayout->addWidget(m_filesButton);
    navLayout->addWidget(m_searchButton);
    navLayout->addWidget(m_reviewButton);
    navLayout->addStretch();
    rootLayout->addWidget(navBar);

    m_stack = new QStackedWidget(this);
    rootLayout->addWidget(m_stack);

    QWidget *filesPage = new QWidget(this);
    QVBoxLayout *filesLayout = new QVBoxLayout(filesPage);
    filesLayout->setContentsMargins(0, 0, 0, 0);
    filesLayout->setSpacing(0);

    QWidget *toolbar = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(4, 4, 4, 4);
    toolbarLayout->setSpacing(4);

    QPushButton *btnNewNote = new QPushButton(toolbar);
    QPushButton *btnNewFolder = new QPushButton(toolbar);
    btnNewNote->setFixedSize(20, 20);
    btnNewFolder->setFixedSize(30, 30);
    btnNewNote->setIcon(QIcon(":/icons/new-file.svg"));
    btnNewFolder->setIcon(QIcon(":/icons/new-folder.svg"));
    btnNewNote->setIconSize(QSize(20, 20));
    btnNewFolder->setIconSize(QSize(20, 20));
    btnNewNote->setToolTip(tr("新建笔记"));
    btnNewFolder->setToolTip(tr("新建文件夹"));
    btnNewNote->setFlat(true);
    btnNewFolder->setFlat(true);
    btnNewNote->setStyleSheet("background: transparent; border: none;");
    btnNewFolder->setStyleSheet("background: transparent; border: none;");
    toolbarLayout->addWidget(btnNewNote);
    toolbarLayout->addWidget(btnNewFolder);
    toolbarLayout->addStretch();
    filesLayout->addWidget(toolbar);

    m_treeView = new QTreeView(this);
    m_model = new QFileSystemModel(this);
    m_sortProxy = new DirFirstSortProxy(this);
    m_sortProxy->setSourceModel(m_model);
    m_sortProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_sortProxy->setDynamicSortFilter(true);
    m_sortProxy->sort(0, Qt::AscendingOrder);
    m_iconProvider = new QbsidianIconProvider();
    m_model->setIconProvider(m_iconProvider);

    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_model->setNameFilters({"*.md"});
    m_model->setNameFilterDisables(false);

    m_treeView->setModel(m_sortProxy);
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);
    m_treeView->setHeaderHidden(true);
    m_treeView->setIndentation(12);
    m_treeView->setDragEnabled(true);
    m_treeView->setDragDropMode(QAbstractItemView::DragOnly);
    m_treeView->setDefaultDropAction(Qt::CopyAction);
    m_treeDelegate = new FileTreeDelegate(m_treeView, m_sortProxy, m_model, this);
    m_treeView->setItemDelegate(m_treeDelegate);
    for (int i = 1; i < m_model->columnCount(); ++i)
        m_treeView->hideColumn(i);

    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    filesLayout->addWidget(m_treeView);

    QWidget *searchPage = new QWidget(this);
    QVBoxLayout *searchLayout = new QVBoxLayout(searchPage);
    searchLayout->setContentsMargins(6, 6, 6, 0);
    searchLayout->setSpacing(6);
    m_searchBox = new QLineEdit(searchPage);
    m_searchBox->setPlaceholderText(tr("搜索笔记内容..."));
    m_searchBox->setClearButtonEnabled(true);
    m_searchResults = new QListWidget(searchPage);
    m_searchResults->setAlternatingRowColors(false);
    m_searchResults->setFrameShape(QFrame::NoFrame);
    m_searchResults->setSpacing(6);
    m_searchResults->setStyleSheet(
        "QListWidget { border: none; background: transparent; }"
        "QListWidget::item { border: none; padding: 0; }"
        "QListWidget::item:selected { background: transparent; }"
    );
    searchLayout->addWidget(m_searchBox);
    searchLayout->addWidget(m_searchResults);

    m_stack->addWidget(filesPage);
    m_stack->addWidget(searchPage);

    m_stack->setCurrentWidget(filesPage);

    connect(m_treeView, &QTreeView::clicked,
            this, &FileExplorerPane::onItemClicked);
    connect(m_treeView, &QWidget::customContextMenuRequested,
            this, &FileExplorerPane::onCustomContextMenu);

    connect(m_searchBox, &QLineEdit::returnPressed,
            this, &FileExplorerPane::onSearchReturnPressed);
    connect(m_searchResults, &QListWidget::itemClicked,
            this, &FileExplorerPane::onSearchResultClicked);

    connect(m_filesButton, &QPushButton::clicked, this, [this, filesPage]() {
        m_stack->setCurrentWidget(filesPage);
        m_filesButton->setChecked(true);
        m_searchButton->setChecked(false);
        m_reviewButton->setChecked(false);
    });
    connect(m_searchButton, &QPushButton::clicked, this, [this, searchPage]() {
        m_stack->setCurrentWidget(searchPage);
        m_filesButton->setChecked(false);
        m_searchButton->setChecked(true);
        m_reviewButton->setChecked(false);
        m_searchBox->setFocus();
    });
    connect(m_reviewButton, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
        m_filesButton->setChecked(false);
        m_searchButton->setChecked(false);
        m_reviewButton->setChecked(true);
        emit reviewTimelineRequested();
    });

    connect(btnNewNote, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        QString name = QInputDialog::getText(this, tr("新建笔记"),
                                             tr("笔记名称（不含扩展名）:"),
                                             QLineEdit::Normal, QString(), &ok);
        if (ok && !name.isEmpty())
            emit newNoteRequested(currentTargetDirectory(), name);
    });
    connect(btnNewFolder, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        QString name = QInputDialog::getText(this, tr("新建文件夹"),
                                             tr("文件夹名称:"),
                                             QLineEdit::Normal, QString(), &ok);
        if (ok && !name.isEmpty())
            emit newFolderRequested(currentTargetDirectory(), name);
    });

    new AutoHideScrollAreaFilter(m_treeView, this);
    new AutoHideScrollAreaFilter(m_searchResults, this);
}

QString FileExplorerPane::rootPath() const
{
    return m_model->rootPath();
}

void FileExplorerPane::setRootPath(const QString &path)
{
    QModelIndex rootIndex = m_model->setRootPath(path);
    m_treeView->setRootIndex(m_sortProxy->mapFromSource(rootIndex));
    m_sortProxy->sort(0, Qt::AscendingOrder);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);
}

void FileExplorerPane::onItemClicked(const QModelIndex &proxyIndex)
{
    QModelIndex sourceIndex = m_sortProxy->mapToSource(proxyIndex);
    QFileInfo info = m_model->fileInfo(sourceIndex);
    if (info.isFile() && info.suffix() == "md")
        emit fileSelected(info.absoluteFilePath());
}

void FileExplorerPane::onCustomContextMenu(const QPoint &pos)
{
    QModelIndex proxyIndex = m_treeView->indexAt(pos);
    QModelIndex sourceIndex;
    QString dirPath;
    if (proxyIndex.isValid()) {
        sourceIndex = m_sortProxy->mapToSource(proxyIndex);
        QFileInfo fileInfo = m_model->fileInfo(sourceIndex);
        dirPath = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.absolutePath();
    } else {
        dirPath = m_model->rootPath();
    }

    QMenu menu(m_treeView);
    QAction *newNoteAction = menu.addAction(tr("新建笔记"));
    QAction *newFolderAction = menu.addAction(tr("新建文件夹"));
    QAction *deleteAction = nullptr;
    if (proxyIndex.isValid()) {
        menu.addSeparator();
        deleteAction = menu.addAction(tr("删除"));
    }

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
    } else if (deleteAction && chosen == deleteAction) {
        QFileInfo info = m_model->fileInfo(sourceIndex);
        emit deleteRequested(info.absoluteFilePath());
    }
}

void FileExplorerPane::setNoteManager(NoteManager *manager)
{
    m_noteManager = manager;
}

void FileExplorerPane::setVaultPath(const QString &path)
{
    m_vaultPath = path;
}

void FileExplorerPane::onSearchReturnPressed()
{
    if (!m_noteManager || m_vaultPath.isEmpty())
        return;

    m_searchResults->clear();
    QString query = m_searchBox->text().trimmed();
    if (query.isEmpty())
        return;

    QRegularExpression regex(query, QRegularExpression::CaseInsensitiveOption);
    if (!regex.isValid())
        return;

    QVector<SearchResult> results = m_noteManager->searchInVault(m_vaultPath, regex);
    if (results.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(m_searchResults);
        QWidget *card = new QWidget(m_searchResults);
        card->setObjectName("searchCard");
        card->setStyleSheet("QWidget#searchCard { background: rgba(128,128,128,0.08); border-radius: 8px; } QLabel { color: #8a8a8a; }");
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 8, 10, 8);
        QLabel *label = new QLabel(tr("没有找到匹配结果"), card);
        cardLayout->addWidget(label);
        item->setSizeHint(card->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        m_searchResults->setItemWidget(item, card);
        return;
    }

    for (const SearchResult &res : results) {
        QFileInfo fi(res.filePath);
        QListWidgetItem *item = new QListWidgetItem(m_searchResults);
        QWidget *card = new QWidget(m_searchResults);
        card->setObjectName("searchCard");
        card->setStyleSheet(
            "QWidget#searchCard { background: rgba(128,128,128,0.08); border-radius: 8px; }"
            "QLabel#searchTitle { font-weight: 600; color: palette(text); }"
            "QLabel#searchExcerpt { color: #8a8a8a; }"
        );
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 8, 10, 8);
        cardLayout->setSpacing(4);

        QLabel *title = new QLabel(fi.fileName(), card);
        title->setObjectName("searchTitle");
        QLabel *excerpt = new QLabel(QString("第 %1 行 · %2").arg(res.lineNumber).arg(res.lineContent), card);
        excerpt->setObjectName("searchExcerpt");
        excerpt->setWordWrap(true);

        cardLayout->addWidget(title);
        cardLayout->addWidget(excerpt);
        item->setSizeHint(card->sizeHint());
        item->setData(Qt::UserRole, res.filePath);
        item->setData(Qt::UserRole + 1, res.lineNumber);
        m_searchResults->setItemWidget(item, card);
    }
}

void FileExplorerPane::onSearchResultClicked(QListWidgetItem *item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    if (filePath.isEmpty())
        return;

    int lineNumber = item->data(Qt::UserRole + 1).toInt();
    emit searchResultClicked(filePath, lineNumber);
}

void FileExplorerPane::setReviewButtonChecked(bool checked)
{
    m_reviewButton->setChecked(checked);
    if (checked) {
        m_filesButton->setChecked(false);
        m_searchButton->setChecked(false);
    }
}

void FileExplorerPane::setDarkMode(bool dark)
{
    if (!m_treeDelegate)
        return;

    m_treeDelegate->setDarkMode(dark);
    m_treeView->viewport()->update();
}

QString FileExplorerPane::currentTargetDirectory() const
{
    QString fallback = m_vaultPath.isEmpty() ? rootPath() : m_vaultPath;
    QModelIndex proxyIndex = m_treeView->currentIndex();
    if (!proxyIndex.isValid())
        return fallback;

    QModelIndex sourceIndex = m_sortProxy->mapToSource(proxyIndex);
    QFileInfo info = m_model->fileInfo(sourceIndex);
    if (!info.exists())
        return fallback;

    return info.isDir() ? info.absoluteFilePath() : info.absolutePath();
}
