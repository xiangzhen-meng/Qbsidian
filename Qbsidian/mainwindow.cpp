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
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_fileExplorer(nullptr)
    , m_editor(nullptr)
    , m_preview(nullptr)
    , m_previewTimer(nullptr)
    , m_isModified(false)
    , m_themeMode(ThemeMode::Light)
{
    ui->setupUi(this);
    showMaximized();

    setupPanes();
    setupMenuBar();
    connectSignals();
    loadThemeSetting();
    applyWindowTheme();
    applyContentTheme();

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

QString MainWindow::buildLightQss() const
{
    return QStringLiteral(
        "QMainWindow { background-color: #ffffff; }"
        "QWidget { font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Helvetica, \"Apple Color Emoji\", Arial, sans-serif; font-size: 14px; color: #222222; }"

        "QMenuBar { background-color: #f6f7f8; color: #222222; border-bottom: 1px solid #ebedf0; padding: 2px 0; }"
        "QMenuBar::item { padding: 4px 12px; background: transparent; border-radius: 4px; }"
        "QMenuBar::item:selected { background-color: #e2e5e9; }"
        "QMenu { background-color: #ffffff; border: 1px solid #ebedf0; border-radius: 8px; padding: 4px; }"
        "QMenu::item { padding: 6px 28px 6px 12px; border-radius: 4px; }"
        "QMenu::item:selected { background-color: #2e80f2; color: #ffffff; }"
        "QMenu::separator { height: 1px; background: #ebedf0; margin: 4px 8px; }"

        "QStatusBar { background-color: #f6f7f8; color: #ababab; border-top: 1px solid #ebedf0; font-size: 12px; }"

        "QSplitter::handle { background-color: #ebedf0; width: 1px; }"
        "QSplitter::handle:hover { background-color: #2e80f2; }"

        "QTreeView { background-color: #f6f7f8; border: none; outline: none; color: #222222; }"
        "QTreeView::item { padding: 3px 8px; border-radius: 4px; min-height: 24px; }"
        "QTreeView::item:hover { background-color: #e2e5e9; }"
        "QTreeView::item:selected { background-color: #d4d4d4; color: #222222; }"
        "QTreeView::branch { background: transparent; }"

        "QPlainTextEdit { background-color: #ffffff; color: #222222; border: none; selection-background-color: rgba(46,128,242,0.18); padding: 12px; }"
        "QPlainTextEdit:focus { border: none; }"

        "QTextBrowser { background-color: #ffffff; color: #222222; border: none; padding: 16px; }"

        "QScrollBar:vertical { background: #f6f7f8; width: 8px; border: none; }"
        "QScrollBar::handle:vertical { background: #d4d4d4; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #bdbdbd; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar:horizontal { background: #f6f7f8; height: 8px; border: none; }"
        "QScrollBar::handle:horizontal { background: #d4d4d4; border-radius: 4px; min-width: 30px; }"
        "QScrollBar::handle:horizontal:hover { background: #bdbdbd; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"

        "QToolTip { background-color: #f6f7f8; color: #222222; border: 1px solid #d4d4d4; border-radius: 4px; padding: 4px 8px; font-size: 12px; }"

        "QMessageBox { background-color: #ffffff; }"
        "QPushButton { background-color: #2e80f2; color: #ffffff; border: none; border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #1a6fd6; }"
        "QPushButton:pressed { background-color: #155ec0; }"

        "QInputDialog { background-color: #ffffff; }"
        "QLineEdit { background-color: #f6f7f8; color: #222222; border: 1px solid #ebedf0; border-radius: 6px; padding: 6px 10px; selection-background-color: rgba(46,128,242,0.18); }"
        "QLineEdit:focus { border-color: #2e80f2; }"
    );
}

QString MainWindow::buildDarkQss() const
{
    return QStringLiteral(
        "QMainWindow { background-color: #1c2127; }"
        "QWidget { font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Helvetica, \"Apple Color Emoji\", Arial, sans-serif; font-size: 14px; color: #dadada; }"

        "QMenuBar { background-color: #181c20; color: #dadada; border-bottom: 1px solid #35393e; padding: 2px 0; }"
        "QMenuBar::item { padding: 4px 12px; background: transparent; border-radius: 4px; }"
        "QMenuBar::item:selected { background-color: #2c313c; }"
        "QMenu { background-color: #1c2127; border: 1px solid #35393e; border-radius: 8px; padding: 4px; }"
        "QMenu::item { padding: 6px 28px 6px 12px; border-radius: 4px; }"
        "QMenu::item:selected { background-color: #2e80f2; color: #ffffff; }"
        "QMenu::separator { height: 1px; background: #35393e; margin: 4px 8px; }"

        "QStatusBar { background-color: #181c20; color: #999999; border-top: 1px solid #35393e; font-size: 12px; }"

        "QSplitter::handle { background-color: #35393e; width: 1px; }"
        "QSplitter::handle:hover { background-color: #2e80f2; }"

        "QTreeView { background-color: #181c20; border: none; outline: none; color: #dadada; }"
        "QTreeView::item { padding: 3px 8px; border-radius: 4px; min-height: 24px; }"
        "QTreeView::item:hover { background-color: #2c313c; }"
        "QTreeView::item:selected { background-color: #3f3f3f; color: #dadada; }"
        "QTreeView::branch { background: transparent; }"

        "QPlainTextEdit { background-color: #1c2127; color: #dadada; border: none; selection-background-color: rgba(46,128,242,0.25); padding: 12px; }"
        "QPlainTextEdit:focus { border: none; }"

        "QTextBrowser { background-color: #1c2127; color: #dadada; border: none; padding: 16px; }"

        "QScrollBar:vertical { background: #181c20; width: 8px; border: none; }"
        "QScrollBar::handle:vertical { background: #3f3f3f; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #555555; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar:horizontal { background: #181c20; height: 8px; border: none; }"
        "QScrollBar::handle:horizontal { background: #3f3f3f; border-radius: 4px; min-width: 30px; }"
        "QScrollBar::handle:horizontal:hover { background: #555555; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"

        "QToolTip { background-color: #181c20; color: #dadada; border: 1px solid #35393e; border-radius: 4px; padding: 4px 8px; font-size: 12px; }"

        "QMessageBox { background-color: #1c2127; }"
        "QPushButton { background-color: #2e80f2; color: #ffffff; border: none; border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #1a6fd6; }"
        "QPushButton:pressed { background-color: #155ec0; }"

        "QInputDialog { background-color: #1c2127; }"
        "QLineEdit { background-color: #2c313c; color: #dadada; border: 1px solid #35393e; border-radius: 6px; padding: 6px 10px; selection-background-color: rgba(46,128,242,0.25); }"
        "QLineEdit:focus { border-color: #2e80f2; }"
    );
}

void MainWindow::applyWindowTheme()
{
    bool dark = (m_themeMode == ThemeMode::Dark);
    setStyleSheet(dark ? buildDarkQss() : buildLightQss());
}

void MainWindow::applyContentTheme()
{
    bool dark = (m_themeMode == ThemeMode::Dark);
    if (m_preview)
        m_preview->setDarkMode(dark);

    if (m_editor) {
        if (dark) {
            m_editor->setThemeColors(QColor("#181c20"), QColor("#666666"));
        } else {
            m_editor->setThemeColors(QColor("#f6f7f8"), QColor("#bdbdbd"));
        }
    }

    if (m_preview && m_editor) {
        if (!m_currentFilePath.isEmpty() || !m_editor->toPlainText().isEmpty())
            m_preview->showHtml(m_editor->toPlainText());
    }
}

void MainWindow::toggleTheme()
{
    m_themeMode = (m_themeMode == ThemeMode::Light) ? ThemeMode::Dark : ThemeMode::Light;
    applyWindowTheme();
    applyContentTheme();
    saveThemeSetting();
}

void MainWindow::loadThemeSetting()
{
    QSettings settings("Qbsidian", "Qbsidian");
    QString theme = settings.value("theme", "light").toString();
    m_themeMode = (theme == "dark") ? ThemeMode::Dark : ThemeMode::Light;
}

void MainWindow::saveThemeSetting()
{
    QSettings settings("Qbsidian", "Qbsidian");
    settings.setValue("theme", (m_themeMode == ThemeMode::Dark) ? "dark" : "light");
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

    connect(ui->actionToggleTheme, &QAction::triggered,
            this, &MainWindow::toggleTheme);

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
