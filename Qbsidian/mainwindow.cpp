#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "fileexplorerpane.h"
#include "editorpane.h"
#include "previewpane.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QSplitter>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_fileExplorer(nullptr)
    , m_editor(nullptr)
    , m_preview(nullptr)
    , m_previewTimer(nullptr)
    , m_isModified(false)
{
    ui->setupUi(this);
    showMaximized();

    setupStyleSheet();
    setupPanes();
    setupMenuBar();
    connectSignals();

    QString vault = promptVaultDirectory();
    if (!vault.isEmpty()) {
        m_vaultPath = vault;
        m_fileExplorer->setRootPath(m_vaultPath);
        setWindowTitle("Qbsidian - " + m_vaultPath);
    }

    m_previewTimer = new QTimer(this);
    m_previewTimer->setSingleShot(true);
    connect(m_previewTimer, &QTimer::timeout, this, &MainWindow::onPreviewTimer);

    ui->statusbar->showMessage(tr("就绪"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStyleSheet()
{
    setStyleSheet(QStringLiteral(
        "QMainWindow {"
        "    background-color: #292d3b;"
        "}"
        "QWidget {"
        "    font-family: \"SF Pro Text\", \"Microsoft YaHei\", sans-serif;"
        "    font-size: 14px;"
        "    color: #dfe4f5;"
        "}"

        "QMenuBar {"
        "    background-color: #222531;"
        "    color: #dfe4f5;"
        "    border-bottom: 1px solid #3b3f52;"
        "    padding: 2px 0;"
        "}"
        "QMenuBar::item {"
        "    padding: 4px 12px;"
        "    background: transparent;"
        "    border-radius: 4px;"
        "}"
        "QMenuBar::item:selected {"
        "    background-color: #52576d;"
        "}"
        "QMenu {"
        "    background-color: #292d3b;"
        "    border: 1px solid #3b3f52;"
        "    border-radius: 8px;"
        "    padding: 4px;"
        "}"
        "QMenu::item {"
        "    padding: 6px 28px 6px 12px;"
        "    border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #7c3aed;"
        "    color: #ffffff;"
        "}"
        "QMenu::separator {"
        "    height: 1px;"
        "    background: #3b3f52;"
        "    margin: 4px 8px;"
        "}"

        "QStatusBar {"
        "    background-color: #222531;"
        "    color: #bcc3dc;"
        "    border-top: 1px solid #3b3f52;"
        "    font-size: 12px;"
        "}"

        "QSplitter::handle {"
        "    background-color: #3b3f52;"
        "    width: 1px;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #7c3aed;"
        "}"

        "QTreeView {"
        "    background-color: #222531;"
        "    border: none;"
        "    outline: none;"
        "    color: #dfe4f5;"
        "}"
        "QTreeView::item {"
        "    padding: 3px 8px;"
        "    border-radius: 4px;"
        "    min-height: 24px;"
        "}"
        "QTreeView::item:hover {"
        "    background-color: #3b3f52;"
        "}"
        "QTreeView::item:selected {"
        "    background-color: #52576d;"
        "    color: #ffffff;"
        "}"
        "QTreeView::branch {"
        "    background: transparent;"
        "}"

        "QPlainTextEdit {"
        "    background-color: #292d3b;"
        "    color: #dfe4f5;"
        "    border: none;"
        "    selection-background-color: #7c3aed;"
        "    padding: 12px;"
        "}"
        "QPlainTextEdit:focus {"
        "    border: none;"
        "}"

        "QTextBrowser {"
        "    background-color: #292d3b;"
        "    color: #dfe4f5;"
        "    border: none;"
        "    padding: 16px;"
        "}"

        "QScrollBar:vertical {"
        "    background: #222531;"
        "    width: 8px;"
        "    border: none;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #52576d;"
        "    border-radius: 4px;"
        "    min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #6c7185;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0;"
        "}"
        "QScrollBar:horizontal {"
        "    background: #222531;"
        "    height: 8px;"
        "    border: none;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #52576d;"
        "    border-radius: 4px;"
        "    min-width: 30px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "    background: #6c7185;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    width: 0;"
        "}"

        "QToolTip {"
        "    background-color: #3b3f52;"
        "    color: #dfe4f5;"
        "    border: 1px solid #52576d;"
        "    border-radius: 4px;"
        "    padding: 4px 8px;"
        "    font-size: 12px;"
        "}"

        "QMessageBox {"
        "    background-color: #292d3b;"
        "}"
        "QPushButton {"
        "    background-color: #7c3aed;"
        "    color: #ffffff;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 6px 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #8b5cf6;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #6d28d9;"
        "}"

        "QInputDialog {"
        "    background-color: #292d3b;"
        "}"
        "QLineEdit {"
        "    background-color: #3b3f52;"
        "    color: #dfe4f5;"
        "    border: 1px solid #52576d;"
        "    border-radius: 6px;"
        "    padding: 6px 10px;"
        "    selection-background-color: #7c3aed;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #7c3aed;"
        "}"
    ));
}

void MainWindow::setupPanes()
{
    m_splitter = ui->splitter;

    m_fileExplorer = new FileExplorerPane(this);
    m_editor = new EditorPane(this);
    m_preview = new PreviewPane(this);

    m_splitter->addWidget(m_fileExplorer);
    m_splitter->addWidget(m_editor);
    m_splitter->addWidget(m_preview);

    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 4);
    m_splitter->setStretchFactor(2, 3);
}

void MainWindow::setupMenuBar()
{
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
}

void MainWindow::connectSignals()
{
    connect(m_fileExplorer, &FileExplorerPane::fileSelected,
            this, &MainWindow::onFileSelected);
    connect(m_fileExplorer, &FileExplorerPane::newNoteRequested,
            this, &MainWindow::onNewNoteRequested);
    connect(m_fileExplorer, &FileExplorerPane::newFolderRequested,
            this, &MainWindow::onNewFolderRequested);

    connect(m_editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::onTextChanged);

    connect(ui->actionSave, &QAction::triggered,
            this, &MainWindow::onSave);

    connect(m_editor, &QPlainTextEdit::undoAvailable,
            ui->actionUndo, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::redoAvailable,
            ui->actionRedo, &QAction::setEnabled);
    connect(ui->actionUndo, &QAction::triggered,
            m_editor, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered,
            m_editor, &QPlainTextEdit::redo);
}

QString MainWindow::promptVaultDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("选择知识库（Vault）目录"),
        QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    return QDir::toNativeSeparators(dir);
}

void MainWindow::updateTitle()
{
    QString title = "Qbsidian - " + m_vaultPath;
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fi(m_currentFilePath);
        title = fi.fileName();
        if (m_isModified)
            title += " *";
        title += " - Qbsidian";
    }
    setWindowTitle(title);
}

void MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->statusbar->showMessage(tr("无法打开文件: %1").arg(path));
        return;
    }
    QByteArray data = file.readAll();
    QString content = QString::fromUtf8(data);
    file.close();

    m_currentFilePath = path;
    m_isModified = false;

    m_editor->setPlainText(content);
    m_preview->showHtml(content);

    updateTitle();
    ui->statusbar->showMessage(tr("已打开: %1").arg(QFileInfo(path).fileName()));
}

void MainWindow::saveFile()
{
    if (m_currentFilePath.isEmpty())
        return;

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, tr("保存失败"), tr("无法写入文件:\n%1").arg(m_currentFilePath));
        return;
    }
    QByteArray data = m_editor->toPlainText().toUtf8();
    file.write(data);
    file.close();

    m_isModified = false;
    updateTitle();
    ui->statusbar->showMessage(tr("已保存"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_isModified) {
        event->accept();
        return;
    }

    QMessageBox::StandardButton btn = QMessageBox::warning(
        this, tr("未保存的修改"),
        tr("当前笔记有未保存的修改。\n\n是否保存后退出？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (btn == QMessageBox::Save) {
        saveFile();
        event->accept();
    } else if (btn == QMessageBox::Discard) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::onFileSelected(const QString &absoluteFilePath)
{
    if (m_isModified) {
        QMessageBox::StandardButton btn = QMessageBox::warning(
            this, tr("未保存的修改"),
            tr("当前笔记有未保存的修改，是否保存？"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (btn == QMessageBox::Save)
            saveFile();
        else if (btn == QMessageBox::Cancel)
            return;
    }

    loadFile(absoluteFilePath);
}

void MainWindow::onTextChanged()
{
    m_isModified = true;
    updateTitle();
    m_previewTimer->start(300);
}

void MainWindow::onPreviewTimer()
{
    m_preview->showHtml(m_editor->toPlainText());
}

void MainWindow::onSave()
{
    saveFile();
}

void MainWindow::onNewNoteRequested(const QString &directory, const QString &baseName)
{
    QFileInfo dirInfo(directory);
    if (!dirInfo.isDir())
        return;

    QString name = baseName;
    QString path = dirInfo.absoluteFilePath() + "/" + name + ".md";
    int counter = 1;
    while (QFile::exists(path)) {
        name = baseName + " (" + QString::number(counter) + ")";
        path = dirInfo.absoluteFilePath() + "/" + name + ".md";
        ++counter;
    }

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.close();
        m_fileExplorer->setRootPath(m_vaultPath);
        ui->statusbar->showMessage(tr("已创建: %1.md").arg(name));
    } else {
        ui->statusbar->showMessage(tr("创建失败: %1").arg(name));
    }
}

void MainWindow::onNewFolderRequested(const QString &directory, const QString &folderName)
{
    QFileInfo dirInfo(directory);
    if (!dirInfo.isDir())
        return;

    QString name = folderName;
    QString path = dirInfo.absoluteFilePath() + "/" + name;
    int counter = 1;
    while (QDir(path).exists()) {
        name = folderName + " (" + QString::number(counter) + ")";
        path = dirInfo.absoluteFilePath() + "/" + name;
        ++counter;
    }

    if (QDir().mkpath(path)) {
        m_fileExplorer->setRootPath(m_vaultPath);
        ui->statusbar->showMessage(tr("已创建文件夹: %1").arg(name));
    } else {
        ui->statusbar->showMessage(tr("创建文件夹失败: %1").arg(name));
    }
}
