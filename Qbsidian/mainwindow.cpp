#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "fileexplorerpane.h"
#include "editorpane.h"
#include "previewpane.h"
#include "reviewtimelinepane.h"
#include "practicedialog.h"
#include "graphpane.h"
#include "aiagent.h"
#include "aiassistantpane.h"
#include "notemanager.h"
#include "ReviewManager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QSplitter>
#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QSettings>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QTextBlock>
#include <QTextCursor>
#include <QSignalBlocker>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QPushButton>
#include <QToolButton>
#include <QDate>
#include <QTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_fileExplorer(nullptr)
    , m_tabWidget(nullptr)
    , m_reviewTimeline(nullptr)
    , m_reviewTimelinePage(nullptr)
    , m_graphPane(nullptr)
    , m_graphPage(nullptr)
    , m_aiAgent(nullptr)
    , m_aiAssistantPane(nullptr)
    , m_noteManager(nullptr)
    , m_reviewManager(nullptr)
    , m_noteManagerHadError(false)
    , m_themeMode(ThemeMode::Light)
    , m_aiPendingAction(AiPendingAction::None)
{
    ui->setupUi(this);
    showMaximized();

    setupPanes();
    setupMenuBar();

    m_noteManager = new NoteManager(this);
    connect(m_noteManager, &NoteManager::errorOccurred,
            this, &MainWindow::onNoteManagerError);

    m_reviewManager = new ReviewManager;
    m_aiAgent = new AIAgent(this);
    QSettings aiSettings("Qbsidian", "Qbsidian");
    m_aiAgent->setApiKey(aiSettings.value("ai/deepseekApiKey").toString());

    connectSignals();

    m_fileExplorer->setNoteManager(m_noteManager);

    loadThemeSetting();
    applyWindowTheme();
    applyContentTheme();

    QString vault = promptVaultDirectory();
    if (!vault.isEmpty()) {
        m_vaultPath = vault;
        m_reviewManager->setVaultPath(m_vaultPath);
        m_fileExplorer->setRootPath(m_vaultPath);
        m_fileExplorer->setVaultPath(m_vaultPath);
        setWindowTitle(QString());
    }
    ui->statusbar->showMessage(tr("就绪"));
}

MainWindow::~MainWindow()
{
    delete m_reviewManager;
    delete ui;
}

QString MainWindow::buildLightQss() const
{
    return QStringLiteral(
        "QMainWindow { background-color: #ffffff; }"
        "QWidget { font-family: \"LXGW WenKai Screen\", \"LXGW WenKai\", -apple-system, BlinkMacSystemFont, \"Segoe UI\", Helvetica, Arial, sans-serif; font-size: 14px; color: #0e0e0e; }"

        "QMenuBar { background-color: #f3f3f3; color: #0e0e0e; border-bottom: 1px solid #dddddd; padding: 3px 0; }"
        "QMenuBar::item { padding: 5px 12px; background: transparent; border-radius: 5px; }"
        "QMenuBar::item:selected { background-color: rgba(129,161,193,0.24); color: #0e0e0e; }"
        "QMenu { background-color: #ffffff; color: #0e0e0e; border: 1px solid #dddddd; border-radius: 8px; padding: 6px; }"
        "QMenu::item { padding: 7px 30px 7px 14px; border-radius: 5px; }"
        "QMenu::item:selected { background-color: #81a1c1; color: #ffffff; }"
        "QMenu::separator { height: 1px; background: #dddddd; margin: 5px 8px; }"

        "QStatusBar { background-color: #f3f3f3; color: #7f7f7f; border-top: 1px solid #dddddd; font-size: 12px; }"

        "QSplitter::handle { background-color: #dddddd; width: 1px; height: 1px; }"
        "QSplitter::handle:hover { background-color: #81a1c1; }"

        "QTabWidget#mainTabs::pane { background-color: #ffffff; border: none; }"
        "QTabWidget#mainTabs::tab-bar { background-color: #ffffff; left: 0px; }"
        "QTabWidget#mainTabs > QWidget { background-color: #ffffff; }"
        "QTabBar#mainTabBar { background-color: #ffffff; border: none; }"
        "QTabBar#mainTabBar::tab { background-color: #ffffff; color: #7f7f7f; padding: 8px 30px 8px 14px; border: none; border-right: 1px solid #dddddd; min-width: 120px; }"
        "QTabBar#mainTabBar::tab:!selected { background-color: #ffffff; }"
        "QTabBar#mainTabBar::tab:selected { background-color: #ffffff; color: #0e0e0e; border-top: 2px solid #81a1c1; }"
        "QTabBar#mainTabBar::tab:hover:!selected { background-color: #ffffff; color: #0e0e0e; }"
        "QToolButton#tabCloseButton { background: transparent; border: none; border-radius: 9px; padding: 0; }"
        "QToolButton#tabCloseButton:hover { background-color: rgba(127,127,127,0.16); }"

        "QTreeView { background-color: #fcfcfc; border: none; outline: none; color: #272727; selection-background-color: rgba(129,161,193,0.24); selection-color: #0e0e0e; }"
        "QTreeView::item { padding: 4px 7px 4px 5px; border-radius: 5px; min-height: 24px; }"
        "QTreeView::item:hover { background-color: rgba(129,161,193,0.12); }"
        "QTreeView::item:selected { background-color: rgba(129,161,193,0.28); color: #0e0e0e; }"
        "QTreeView::branch { background: transparent; }"

        "QPlainTextEdit { background-color: #ffffff; color: #0e0e0e; border: none; font-size: 18px; selection-background-color: rgba(129,161,193,0.24); padding: 24px; }"
        "QPlainTextEdit:focus { border: none; }"

        "QTextBrowser { background-color: #ffffff; color: #0e0e0e; border: none; font-size: 18px; padding: 24px; }"

        "QListWidget { background-color: transparent; color: #0e0e0e; }"
        "QListWidget::item:hover { background-color: rgba(129,161,193,0.10); }"

        "QScrollBar:vertical { width: 5px; border: none; background: transparent; }"
        "QScrollBar::handle:vertical { background: #dcdcdc; border-radius: 2px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #c8d4e0; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar:horizontal { height: 5px; border: none; background: transparent; }"
        "QScrollBar::handle:horizontal { background: #dcdcdc; border-radius: 2px; min-width: 30px; }"
        "QScrollBar::handle:horizontal:hover { background: #c8d4e0; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"

        "QToolTip { background-color: #f3f3f3; color: #0e0e0e; border: 1px solid #dddddd; border-radius: 4px; padding: 4px 8px; font-size: 12px; }"

        "QMessageBox { background-color: #ffffff; }"
        "QDialog { background-color: #ffffff; }"
        "QPushButton { background-color: #81a1c1; color: #ffffff; border: none; border-radius: 6px; padding: 7px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #5e81ac; }"
        "QPushButton:pressed { background-color: #4c6f99; }"
        "QPushButton:flat { background: transparent; color: #5e81ac; }"
        "QPushButton:flat:hover { background: rgba(129,161,193,0.12); }"
        "QPushButton:checked { background-color: #81a1c1; color: #ffffff; }"

        "QInputDialog { background-color: #ffffff; }"
        "QLineEdit, QComboBox { background-color: #fcfcfc; color: #0e0e0e; border: 1px solid #dddddd; border-radius: 6px; padding: 7px 10px; selection-background-color: rgba(129,161,193,0.24); }"
        "QLineEdit:focus, QComboBox:focus { border-color: #81a1c1; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox QAbstractItemView { background: #ffffff; color: #0e0e0e; selection-background-color: rgba(129,161,193,0.24); border: 1px solid #dddddd; outline: none; }"
        "QCheckBox { spacing: 7px; }"
        "QCheckBox::indicator { width: 14px; height: 14px; border: 1px solid #9c9c9c; border-radius: 3px; background: #ffffff; }"
        "QCheckBox::indicator:checked { background: #81a1c1; border-color: #81a1c1; }"
    );
}

QString MainWindow::buildDarkQss() const
{
    return QStringLiteral(
        "QMainWindow { background-color: #2E3440; }"
        "QWidget { font-family: \"LXGW WenKai Screen\", \"LXGW WenKai\", -apple-system, BlinkMacSystemFont, \"Segoe UI\", Helvetica, Arial, sans-serif; font-size: 14px; color: #d8dee9; }"

        "QMenuBar { background-color: #434C5E; color: #d8dee9; border-bottom: 1px solid #2E3440; padding: 3px 0; }"
        "QMenuBar::item { padding: 5px 12px; background: transparent; border-radius: 5px; }"
        "QMenuBar::item:selected { background-color: rgba(129,161,193,0.22); color: #eceff4; }"
        "QMenu { background-color: #3B4252; color: #d8dee9; border: 1px solid #4C566A; border-radius: 8px; padding: 6px; }"
        "QMenu::item { padding: 7px 30px 7px 14px; border-radius: 5px; }"
        "QMenu::item:selected { background-color: #81a1c1; color: #2E3440; }"
        "QMenu::separator { height: 1px; background: #4C566A; margin: 5px 8px; }"

        "QStatusBar { background-color: #434C5E; color: #e5e9f0; border-top: 1px solid #2E3440; font-size: 12px; }"

        "QSplitter::handle { background-color: #2E3440; width: 1px; height: 1px; }"
        "QSplitter::handle:hover { background-color: #81a1c1; }"

        "QTabWidget#mainTabs::pane { background-color: #2E3440; border: none; }"
        "QTabWidget#mainTabs::tab-bar { background-color: #2E3440; left: 0px; }"
        "QTabWidget#mainTabs > QWidget { background-color: #2E3440; }"
        "QTabBar#mainTabBar { background-color: #2E3440; border: none; }"
        "QTabBar#mainTabBar::tab { background-color: #2E3440; color: #d8dee9; padding: 8px 30px 8px 14px; border: none; border-right: 1px solid #2E3440; min-width: 120px; }"
        "QTabBar#mainTabBar::tab:!selected { background-color: #2E3440; }"
        "QTabBar#mainTabBar::tab:selected { background-color: #2E3440; color: #eceff4; border-top: 2px solid #81a1c1; }"
        "QTabBar#mainTabBar::tab:hover:!selected { background-color: #2E3440; color: #eceff4; }"
        "QToolButton#tabCloseButton { background: transparent; border: none; border-radius: 9px; padding: 0; }"
        "QToolButton#tabCloseButton:hover { background-color: rgba(216,222,233,0.16); }"

        "QTreeView { background-color: #3B4252; border: none; outline: none; color: #d8dee9; selection-background-color: rgba(129,161,193,0.24); selection-color: #eceff4; }"
        "QTreeView::item { padding: 4px 7px 4px 5px; border-radius: 5px; min-height: 24px; }"
        "QTreeView::item:hover { background-color: rgba(129,161,193,0.14); }"
        "QTreeView::item:selected { background-color: rgba(129,161,193,0.32); color: #eceff4; }"
        "QTreeView::branch { background: transparent; }"

        "QPlainTextEdit { background-color: #2E3440; color: #d8dee9; border: none; font-size: 18px; selection-background-color: rgba(129,161,193,0.24); padding: 24px; }"
        "QPlainTextEdit:focus { border: none; }"

        "QTextBrowser { background-color: #2E3440; color: #d8dee9; border: none; font-size: 18px; padding: 24px; }"

        "QListWidget { background-color: transparent; color: #d8dee9; }"
        "QListWidget::item:hover { background-color: rgba(129,161,193,0.10); }"

        "QScrollBar:vertical { width: 5px; border: none; background: transparent; }"
        "QScrollBar::handle:vertical { background: rgba(216,222,233,0.26); border-radius: 2px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: rgba(216,222,233,0.42); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar:horizontal { height: 5px; border: none; background: transparent; }"
        "QScrollBar::handle:horizontal { background: rgba(216,222,233,0.26); border-radius: 2px; min-width: 30px; }"
        "QScrollBar::handle:horizontal:hover { background: rgba(216,222,233,0.42); }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"

        "QToolTip { background-color: #434C5E; color: #d8dee9; border: 1px solid #4C566A; border-radius: 4px; padding: 4px 8px; font-size: 12px; }"

        "QMessageBox { background-color: #2E3440; }"
        "QDialog { background-color: #2E3440; }"
        "QPushButton { background-color: #81a1c1; color: #2E3440; border: none; border-radius: 6px; padding: 7px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #88C0D0; }"
        "QPushButton:pressed { background-color: #5e81ac; color: #eceff4; }"
        "QPushButton:flat { background: transparent; color: #88C0D0; }"
        "QPushButton:flat:hover { background: rgba(129,161,193,0.14); }"
        "QPushButton:checked { background-color: #81a1c1; color: #2E3440; }"

        "QInputDialog { background-color: #2E3440; }"
        "QLineEdit, QComboBox { background-color: #3B4252; color: #d8dee9; border: 1px solid #4C566A; border-radius: 6px; padding: 7px 10px; selection-background-color: rgba(129,161,193,0.24); }"
        "QLineEdit:focus, QComboBox:focus { border-color: #81a1c1; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox QAbstractItemView { background: #3B4252; color: #d8dee9; selection-background-color: rgba(129,161,193,0.24); border: 1px solid #4C566A; outline: none; }"
        "QCheckBox { spacing: 7px; }"
        "QCheckBox::indicator { width: 14px; height: 14px; border: 1px solid #d8dee9; border-radius: 3px; background: #3B4252; }"
        "QCheckBox::indicator:checked { background: #81a1c1; border-color: #81a1c1; }"
    );
}

void MainWindow::applyWindowTheme()
{
    bool dark = (m_themeMode == ThemeMode::Dark);
    setStyleSheet(dark ? buildDarkQss() : buildLightQss());
    if (m_tabWidget && m_tabWidget->tabBar()) {
        m_tabWidget->tabBar()->setStyleSheet(dark
            ? QStringLiteral(
                "QTabBar#mainTabBar { background-color: #2E3440; border: none; }"
                "QTabBar#mainTabBar::tab { background-color: #2E3440; color: #d8dee9; padding: 8px 30px 8px 14px; border: none; border-right: 1px solid #2E3440; min-width: 120px; }"
                "QTabBar#mainTabBar::tab:!selected { background-color: #2E3440; }"
                "QTabBar#mainTabBar::tab:selected { background-color: #2E3440; color: #eceff4; border-top: 2px solid #81a1c1; }"
                "QTabBar#mainTabBar::tab:hover:!selected { background-color: #2E3440; color: #eceff4; }"
                "QToolButton#tabCloseButton { background: transparent; border: none; border-radius: 9px; padding: 0; }"
                "QToolButton#tabCloseButton:hover { background-color: rgba(216,222,233,0.16); }")
            : QStringLiteral(
                "QTabBar#mainTabBar { background-color: #ffffff; border: none; }"
                "QTabBar#mainTabBar::tab { background-color: #ffffff; color: #7f7f7f; padding: 8px 30px 8px 14px; border: none; border-right: 1px solid #dddddd; min-width: 120px; }"
                "QTabBar#mainTabBar::tab:!selected { background-color: #ffffff; }"
                "QTabBar#mainTabBar::tab:selected { background-color: #ffffff; color: #0e0e0e; border-top: 2px solid #81a1c1; }"
                "QTabBar#mainTabBar::tab:hover:!selected { background-color: #ffffff; color: #0e0e0e; }"
                "QToolButton#tabCloseButton { background: transparent; border: none; border-radius: 9px; padding: 0; }"
                "QToolButton#tabCloseButton:hover { background-color: rgba(127,127,127,0.16); }"));
    }
}

void MainWindow::applyContentTheme()
{
    bool dark = (m_themeMode == ThemeMode::Dark);
    m_fileExplorer->setDarkMode(dark);
    if (m_reviewTimeline)
        m_reviewTimeline->setDarkMode(dark);
    if (m_graphPane)
        m_graphPane->setDarkMode(dark);
    if (m_aiAssistantPane)
        m_aiAssistantPane->setDarkMode(dark);
    for (NoteTab *tab : m_tabsByPath) {
        tab->preview->setDarkMode(dark);
        tab->preview->showHtml(tab->editor->toPlainText());
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
    m_tabWidget = new QTabWidget(this);
    m_aiAssistantPane = new AIAssistantPane(this);
    m_tabWidget->setObjectName("mainTabs");
    m_tabWidget->setTabsClosable(false);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(false);
    m_tabWidget->tabBar()->setObjectName("mainTabBar");
    m_tabWidget->tabBar()->setDrawBase(false);
    m_tabWidget->tabBar()->setExpanding(false);
    m_tabWidget->tabBar()->setAutoFillBackground(true);

    m_splitter->addWidget(m_fileExplorer);
    m_splitter->addWidget(m_tabWidget);
    m_splitter->addWidget(m_aiAssistantPane);

    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 7);
    m_splitter->setStretchFactor(2, 2);
    m_aiAssistantPane->setVisible(false);
    m_splitter->setSizes({220, 1200, 0});
}

void MainWindow::setupMenuBar()
{
    ui->actionSave->setIcon(QIcon(":/icons/save.svg"));
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    QMenu *practiceMenu = menuBar()->addMenu(tr("练习(&P)"));
    QAction *startPracticeAction = practiceMenu->addAction(tr("开始抽查练习"));
    connect(startPracticeAction, &QAction::triggered,
            this, &MainWindow::onPracticeRequested);
}

void MainWindow::connectSignals()
{
    connect(m_fileExplorer, &FileExplorerPane::fileSelected,
            this, &MainWindow::onFileSelected);
    connect(m_fileExplorer, &FileExplorerPane::newNoteRequested,
            this, &MainWindow::onNewNoteRequested);
    connect(m_fileExplorer, &FileExplorerPane::newFolderRequested,
            this, &MainWindow::onNewFolderRequested);
    connect(m_fileExplorer, &FileExplorerPane::deleteRequested,
            this, &MainWindow::onDeleteRequested);
    connect(m_fileExplorer, &FileExplorerPane::searchResultClicked,
            this, &MainWindow::onSearchResultClicked);
    connect(m_fileExplorer, &FileExplorerPane::reviewTimelineRequested,
            this, &MainWindow::onReviewTimelineRequested);
    connect(m_fileExplorer, &FileExplorerPane::reviewStrategyRequested,
            this, &MainWindow::onReviewStrategyRequested);
    connect(m_fileExplorer, &FileExplorerPane::folderReviewStrategyRequested,
            this, &MainWindow::onFolderReviewStrategyRequested);
    connect(m_fileExplorer, &FileExplorerPane::practiceRequested,
            this, &MainWindow::onPracticeRequested);
    connect(m_fileExplorer, &FileExplorerPane::graphRequested,
             this, &MainWindow::onGraphRequested);
    connect(m_fileExplorer, &FileExplorerPane::aiAssistantRequested,
            this, &MainWindow::onAiAssistantRequested);
    connect(m_fileExplorer, &FileExplorerPane::renameRequested,
             this, &MainWindow::onRenameRequested);

    connect(m_aiAssistantPane, &AIAssistantPane::closeRequested, this, [this]() {
        showAiAssistant(false);
    });
    connect(m_aiAssistantPane, &AIAssistantPane::questionSubmitted,
            this, &MainWindow::onAiQuestionSubmitted);
    connect(m_aiAssistantPane, &AIAssistantPane::settingsRequested,
            this, &MainWindow::onAiSettingsRequested);
    connect(m_aiAssistantPane, &AIAssistantPane::summarizeRequested,
            this, &MainWindow::onAiSummaryRequested);
    connect(m_aiAssistantPane, &AIAssistantPane::quizRequested,
            this, &MainWindow::onAiQuizRequested);
    connect(m_aiAgent, &AIAgent::requestStarted, this, [this]() {
        m_aiAssistantPane->setBusy(true);
        m_aiAssistantPane->appendSystemMessage(tr("AI 正在思考..."));
    });
    connect(m_aiAgent, &AIAgent::responseReceived,
            this, &MainWindow::onAiResponseReceived);
    connect(m_aiAgent, &AIAgent::errorOccurred,
            this, &MainWindow::onAiErrorOccurred);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onCurrentTabChanged);

    connect(ui->actionSave, &QAction::triggered,
            this, &MainWindow::onSave);

    connect(ui->actionToggleTheme, &QAction::triggered,
            this, &MainWindow::toggleTheme);

    connect(ui->actionUndo, &QAction::triggered, this, [this]() {
        if (NoteTab *tab = currentTab())
            tab->editor->undo();
    });
    connect(ui->actionRedo, &QAction::triggered, this, [this]() {
        if (NoteTab *tab = currentTab())
            tab->editor->redo();
    });
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
    setWindowTitle(QString());
}

MainWindow::NoteTab *MainWindow::currentTab() const
{
    QWidget *page = m_tabWidget->currentWidget();
    if (!page)
        return nullptr;

    for (NoteTab *tab : m_tabsByPath) {
        if (tab->page == page)
            return tab;
    }
    return nullptr;
}

void MainWindow::updateTabTitle(NoteTab *tab)
{
    int index = m_tabWidget->indexOf(tab->page);
    if (index < 0)
        return;

    QString title = QFileInfo(tab->filePath).fileName();
    if (tab->isModified)
        title += " *";
    m_tabWidget->setTabText(index, title);
}

void MainWindow::installTabCloseButton(int index, QWidget *page)
{
    QToolButton *button = new QToolButton(m_tabWidget->tabBar());
    button->setObjectName("tabCloseButton");
    button->setAutoRaise(true);
    button->setCursor(Qt::ArrowCursor);
    button->setIcon(QIcon(":/icons/tab-close.svg"));
    button->setIconSize(QSize(15, 15));
    button->setFixedSize(18, 18);
    button->setToolTip(tr("关闭"));

    connect(button, &QToolButton::clicked, this, [this, page]() {
        int currentIndex = m_tabWidget->indexOf(page);
        if (currentIndex >= 0)
            onTabCloseRequested(currentIndex);
    });

    m_tabWidget->tabBar()->setTabButton(index, QTabBar::RightSide, button);
}

bool MainWindow::openFileInTab(const QString &path)
{
    if (m_tabsByPath.contains(path)) {
        NoteTab *tab = m_tabsByPath.value(path);
        m_tabWidget->setCurrentWidget(tab->page);
        updateTitle();
        return true;
    }

    m_noteManagerHadError = false;
    QString content = m_noteManager->load(path);
    if (m_noteManagerHadError)
        return false;

    NoteTab *tab = new NoteTab;
    tab->page = new QWidget(m_tabWidget);
    tab->splitter = new QSplitter(Qt::Horizontal, tab->page);
    tab->editor = new EditorPane(tab->splitter);
    tab->preview = new PreviewPane(tab->splitter);
    tab->previewTimer = new QTimer(tab->page);
    tab->filePath = path;
    tab->isModified = false;

    QVBoxLayout *layout = new QVBoxLayout(tab->page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tab->splitter);

    tab->splitter->addWidget(tab->editor);
    tab->splitter->addWidget(tab->preview);
    tab->splitter->setStretchFactor(0, 4);
    tab->splitter->setStretchFactor(1, 3);
    tab->splitter->setSizes({520, 700});

    tab->preview->setDarkMode(m_themeMode == ThemeMode::Dark);
    tab->previewTimer->setSingleShot(true);

    {
        QSignalBlocker blocker(tab->editor);
        tab->editor->setPlainText(content);
    }
    tab->preview->showHtml(content);

    connect(tab->editor, &QPlainTextEdit::textChanged, this, [this, tab]() {
        tab->isModified = true;
        updateTabTitle(tab);
        updateTitle();
        tab->previewTimer->start(300);
    });
    connect(tab->previewTimer, &QTimer::timeout, this, [tab]() {
        tab->preview->showHtml(tab->editor->toPlainText());
    });
    connect(tab->preview, &QTextBrowser::anchorClicked,
            this, &MainWindow::onAnchorClicked);
    connect(tab->editor, &QPlainTextEdit::undoAvailable, this, [this, tab](bool available) {
        if (currentTab() == tab)
            ui->actionUndo->setEnabled(available);
    });
    connect(tab->editor, &QPlainTextEdit::redoAvailable, this, [this, tab](bool available) {
        if (currentTab() == tab)
            ui->actionRedo->setEnabled(available);
    });

    int index = m_tabWidget->addTab(tab->page, QFileInfo(path).fileName());
    installTabCloseButton(index, tab->page);
    m_tabsByPath.insert(path, tab);
    m_tabWidget->setCurrentIndex(index);
    m_tabWidget->setTabBarAutoHide(false);
    updateTitle();
    ui->statusbar->showMessage(tr("已打开: %1").arg(QFileInfo(path).fileName()));
    return true;
}

bool MainWindow::saveTab(NoteTab *tab)
{
    if (!tab || tab->filePath.isEmpty())
        return true;

    m_noteManagerHadError = false;
    if (!m_noteManager->save(tab->filePath, tab->editor->toPlainText()))
        return false;

    tab->isModified = false;
    updateTabTitle(tab);
    updateTitle();
    ui->statusbar->showMessage(tr("已保存"));
    return true;
}

bool MainWindow::saveCurrentTab()
{
    return saveTab(currentTab());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool hasModified = false;
    for (NoteTab *tab : m_tabsByPath) {
        if (tab->isModified) {
            hasModified = true;
            break;
        }
    }

    if (!hasModified) {
        event->accept();
        return;
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("未保存的修改"));
    box.setText(tr("存在未保存的标签页，关闭程序前要如何处理？"));
    QPushButton *saveAllButton = box.addButton(tr("全部保存"), QMessageBox::AcceptRole);
    QPushButton *discardAllButton = box.addButton(tr("全部不保存"), QMessageBox::DestructiveRole);
    box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.setDefaultButton(saveAllButton);
    box.exec();

    if (box.clickedButton() == saveAllButton) {
        for (NoteTab *tab : m_tabsByPath) {
            if (tab->isModified && !saveTab(tab)) {
                event->ignore();
                return;
            }
        }
        event->accept();
    } else if (box.clickedButton() == discardAllButton) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::onFileSelected(const QString &absoluteFilePath)
{
    m_fileExplorer->setReviewButtonChecked(false);
    openFileInTab(absoluteFilePath);
}

void MainWindow::onSave()
{
    saveCurrentTab();
}

void MainWindow::onNoteManagerError(const QString &operation, const QString &reason)
{
    m_noteManagerHadError = true;
    ui->statusbar->showMessage(tr("%1: %2").arg(operation, reason));
}

void MainWindow::onNewNoteRequested(const QString &directory, const QString &baseName)
{
    QString path = m_noteManager->createNewNote(directory, baseName);
    if (!path.isEmpty()) {
        m_fileExplorer->setRootPath(m_vaultPath);
        openFileInTab(path);
        QFileInfo fi(path);
        ui->statusbar->showMessage(tr("已创建: %1").arg(fi.fileName()));
    }
}

void MainWindow::onNewFolderRequested(const QString &directory, const QString &folderName)
{
    QString path = m_noteManager->createNewFolder(directory, folderName);
    if (!path.isEmpty()) {
        m_fileExplorer->setRootPath(m_vaultPath);
        QFileInfo fi(path);
        ui->statusbar->showMessage(tr("已创建文件夹: %1").arg(fi.fileName()));
    }
}

void MainWindow::onDeleteRequested(const QString &absolutePath)
{
    QFileInfo info(absolutePath);
    QString typeName = info.isDir() ? tr("文件夹") : tr("笔记");
    QString displayName = info.fileName();

    QMessageBox::StandardButton btn = QMessageBox::warning(
        this, tr("确认删除"),
        tr("确定要删除%1 \"%2\" 吗？\n\n此操作不可撤销。").arg(typeName, displayName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (btn != QMessageBox::Yes)
        return;

    QStringList reviewPathsToRemove;
    if (info.isDir()) {
        QDirIterator it(absolutePath, {"*.md"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            reviewPathsToRemove.append(it.filePath());
        }
    } else if (info.suffix() == "md") {
        reviewPathsToRemove.append(absolutePath);
    }

    if (!m_noteManager->deleteItem(absolutePath))
        return;

    for (const QString &path : reviewPathsToRemove)
        m_reviewManager->removeNoteRecord(path);

    QList<NoteTab *> removedTabs;
    for (NoteTab *tab : m_tabsByPath) {
        if (tab->filePath == absolutePath || tab->filePath.startsWith(absolutePath + "/"))
            removedTabs.append(tab);
    }
    for (NoteTab *tab : removedTabs) {
        int index = m_tabWidget->indexOf(tab->page);
        if (index >= 0)
            closeTabAt(index);
    }

    m_fileExplorer->setRootPath(m_vaultPath);
    ui->statusbar->showMessage(tr("已删除: %1").arg(displayName));
}

void MainWindow::onAnchorClicked(const QUrl &url)
{
    if (url.scheme() == "http" || url.scheme() == "https") {
        QDesktopServices::openUrl(url);
        return;
    }

    if (url.scheme() == "internal") {
        QUrlQuery query(url);
        QString noteName = query.queryItemValue("note", QUrl::FullyDecoded);
        if (noteName.isEmpty())
            noteName = QUrl::fromPercentEncoding(url.host().toUtf8());
        QString foundPath = m_noteManager->findNotePath(m_vaultPath, noteName);
        if (!foundPath.isEmpty()) {
            openFileInTab(foundPath);
            return;
        }

        QMessageBox::StandardButton btn = QMessageBox::question(
            this, tr("笔记不存在"),
            tr("笔记 \"%1\" 不存在，是否立即创建？").arg(noteName),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);

        if (btn == QMessageBox::Yes) {
            QString newPath = m_noteManager->createNewNote(m_vaultPath, noteName);
            if (!newPath.isEmpty()) {
                m_fileExplorer->setRootPath(m_vaultPath);
                openFileInTab(newPath);
            }
        }
        return;
    }
}

void MainWindow::onSearchResultClicked(const QString &filePath, int lineNumber)
{
    if (!openFileInTab(filePath))
        return;

    jumpCurrentTabToLine(lineNumber);
}

void MainWindow::jumpCurrentTabToLine(int lineNumber)
{
    NoteTab *tab = currentTab();
    if (!tab)
        return;

    if (lineNumber <= 1)
        return;

    QTextBlock block = tab->editor->document()->findBlockByNumber(lineNumber - 1);
    if (!block.isValid())
        return;

    QTextCursor cursor(block);
    tab->editor->setTextCursor(cursor);
    tab->editor->centerCursor();
}

void MainWindow::onTabCloseRequested(int index)
{
    NoteTab *tab = nullptr;
    QWidget *page = m_tabWidget->widget(index);
    if (page == m_reviewTimelinePage || page == m_graphPage) {
        closeTabAt(index);
        return;
    }

    for (NoteTab *candidate : m_tabsByPath) {
        if (candidate->page == page) {
            tab = candidate;
            break;
        }
    }
    if (!tab)
        return;

    if (tab->isModified) {
        QMessageBox::StandardButton btn = QMessageBox::warning(
            this, tr("未保存的修改"),
            tr("标签页 \"%1\" 有未保存的修改。\n\n是否保存后关闭？")
                .arg(QFileInfo(tab->filePath).fileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (btn == QMessageBox::Save) {
            if (!saveTab(tab))
                return;
        } else if (btn == QMessageBox::Cancel) {
            return;
        }
    }

    closeTabAt(index);
}

void MainWindow::closeTabAt(int index)
{
    QWidget *page = m_tabWidget->widget(index);
    if (!page)
        return;

    if (page == m_reviewTimelinePage) {
        m_tabWidget->removeTab(index);
        m_reviewTimelinePage->deleteLater();
        m_reviewTimelinePage = nullptr;
        m_reviewTimeline = nullptr;
        m_fileExplorer->setReviewButtonChecked(false);
        updateTitle();
        return;
    }

    if (page == m_graphPage) {
        m_tabWidget->removeTab(index);
        m_graphPage->deleteLater();
        m_graphPage = nullptr;
        m_graphPane = nullptr;
        updateTitle();
        return;
    }

    NoteTab *tab = nullptr;
    for (NoteTab *candidate : m_tabsByPath) {
        if (candidate->page == page) {
            tab = candidate;
            break;
        }
    }
    if (!tab)
        return;

    m_tabsByPath.remove(tab->filePath);
    m_tabWidget->removeTab(index);
    delete tab->page;
    delete tab;
    updateTitle();
}

void MainWindow::onCurrentTabChanged(int index)
{
    Q_UNUSED(index)
    NoteTab *tab = currentTab();
    ui->actionUndo->setEnabled(tab && tab->editor->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(tab && tab->editor->document()->isRedoAvailable());
    updateTitle();
}

void MainWindow::onReviewItemOpenRequested(const QString &noteId)
{
    if (!QFileInfo::exists(noteId)) {
        QMessageBox::warning(this, tr("文件不存在"),
            tr("笔记文件 \"%1\" 不存在，可能已被删除。\n\n将从复习计划中移除。").arg(noteId));
        m_reviewManager->removeNoteRecord(noteId);
        refreshReviewTimeline();
        return;
    }
    m_fileExplorer->setReviewButtonChecked(false);
    openFileInTab(noteId);
}

void MainWindow::onTimelineNotePreviewRequested(const ReviewPlanItem &item)
{
    if (!QFileInfo::exists(item.noteId)) {
        QMessageBox::warning(this, tr("文件不存在"),
            tr("笔记文件 \"%1\" 不存在，可能已被删除。\n\n将从复习计划中移除。").arg(item.noteId));
        m_reviewManager->removeNoteRecord(item.noteId);
        refreshReviewTimeline();
        return;
    }

    m_noteManagerHadError = false;
    QString content = m_noteManager->load(item.noteId);
    if (m_noteManagerHadError)
        return;

    if (m_reviewTimeline)
        m_reviewTimeline->showPreview(item, content);
}

void MainWindow::onTimelineRememberedRequested(const ReviewPlanItem &item)
{
    QFileInfo info(item.noteId);
    if (!info.exists() || !info.isFile())
        return;

    if (item.source == ReviewPlanItemSource::ManualSchedule) {
        m_reviewManager->removeManualSchedule(item.id);
        ui->statusbar->showMessage(tr("已完成手动复习: %1").arg(info.completeBaseName()));
    } else {
        m_reviewManager->processReviewFeedback(item.noteId, true);
        ui->statusbar->showMessage(tr("已按当前策略安排下次复习: %1").arg(info.completeBaseName()));
    }

    refreshReviewTimeline();
    if (m_reviewTimeline)
        m_reviewTimeline->clearPreview();
}

void MainWindow::onTimelineForgottenRequested(const ReviewPlanItem &item)
{
    QFileInfo info(item.noteId);
    if (!info.exists() || !info.isFile())
        return;

    if (item.source == ReviewPlanItemSource::ManualSchedule) {
        m_reviewManager->removeManualSchedule(item.id);
        m_reviewManager->addManualReviewSchedule(item.noteId, info.completeBaseName(),
            QDateTime(QDate::currentDate().addDays(1), QTime(9, 0)));
        ui->statusbar->showMessage(tr("已安排明天再次手动复习: %1").arg(info.completeBaseName()));
    } else {
        m_reviewManager->processReviewFeedback(item.noteId, false);
        ui->statusbar->showMessage(tr("已按当前策略重新安排复习: %1").arg(info.completeBaseName()));
    }

    refreshReviewTimeline();
    if (m_reviewTimeline)
        m_reviewTimeline->clearPreview();
}

void MainWindow::onTimelineManualDeleteRequested(const ReviewPlanItem &item)
{
    if (item.source != ReviewPlanItemSource::ManualSchedule)
        return;

    m_reviewManager->removeManualSchedule(item.id);
    refreshReviewTimeline();
    if (m_reviewTimeline)
        m_reviewTimeline->clearPreview();
    ui->statusbar->showMessage(tr("已删除手动排期: %1").arg(QFileInfo(item.noteId).completeBaseName()));
}

void MainWindow::onTimelineStrategyDateDeleteRequested(const ReviewPlanItem &item)
{
    if (item.source != ReviewPlanItemSource::Strategy)
        return;

    m_reviewManager->excludeStrategyReviewDate(item.noteId, item.reviewTime.date());
    refreshReviewTimeline();
    if (m_reviewTimeline)
        m_reviewTimeline->clearPreview();
    ui->statusbar->showMessage(tr("已删除当天复习规划: %1").arg(QFileInfo(item.noteId).completeBaseName()));
}

void MainWindow::onTimelineStrategyAdjustRequested(const ReviewPlanItem &item)
{
    QFileInfo info(item.noteId);
    if (!info.exists() || !info.isFile())
        return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("调整复习策略"));

    QFormLayout *layout = new QFormLayout(&dialog);
    layout->setVerticalSpacing(12);
    QComboBox *strategyCombo = new QComboBox(&dialog);
    strategyCombo->setStyleSheet(
        "QComboBox { padding: 8px 12px; min-height: 36px; }"
        "QComboBox QAbstractItemView::item { min-height: 42px; padding: 10px 12px; }"
    );
    strategyCombo->addItem(tr("艾宾浩斯"), QStringLiteral("standard_ebbinghaus"));
    strategyCombo->addItem(tr("固定间隔 + 休息日"), QStringLiteral("custom"));
    strategyCombo->addItem(tr("从复习规划中删除"), QStringLiteral("remove_strategy"));
    layout->addRow(tr("策略"), strategyCombo);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString strategyId = strategyCombo->currentData().toString();
    QString title = info.completeBaseName();
    if (strategyId == QStringLiteral("remove_strategy")) {
        if (item.source == ReviewPlanItemSource::Strategy) {
            m_reviewManager->removeStrategyReviewRecord(item.noteId);
        }
        refreshReviewTimeline();
        if (m_reviewTimeline)
            m_reviewTimeline->clearPreview();
        ui->statusbar->showMessage(tr("已从复习规划中删除: %1").arg(title));
        return;
    }

    if (strategyId == QStringLiteral("custom")) {
        strategyId = promptFixedIntervalStrategy();
        if (strategyId.isEmpty())
            return;
    }

    if (item.source == ReviewPlanItemSource::ManualSchedule)
        m_reviewManager->removeManualSchedule(item.id);
    applyReviewStrategy(item.noteId, title, strategyId);
    refreshReviewTimeline();
    ui->statusbar->showMessage(tr("已调整复习策略: %1").arg(title));
}

QString MainWindow::promptFixedIntervalStrategy()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("固定间隔策略"));

    QFormLayout *layout = new QFormLayout(&dialog);
    layout->setVerticalSpacing(12);
    QLineEdit *intervalEdit = new QLineEdit(QStringLiteral("1,3,7,15,30"), &dialog);
    layout->addRow(tr("间隔天数"), intervalEdit);

    QWidget *restDaysWidget = new QWidget(&dialog);
    QHBoxLayout *restDaysLayout = new QHBoxLayout(restDaysWidget);
    restDaysLayout->setContentsMargins(0, 0, 0, 0);
    restDaysLayout->setSpacing(10);
    QList<QCheckBox *> restDayChecks;
    const QStringList dayNames = { tr("一"), tr("二"), tr("三"), tr("四"), tr("五"), tr("六"), tr("日") };
    for (int i = 0; i < dayNames.size(); ++i) {
        QCheckBox *check = new QCheckBox(dayNames.at(i), restDaysWidget);
        check->setProperty("day", i + 1);
        restDayChecks.append(check);
        restDaysLayout->addWidget(check);
    }
    layout->addRow(tr("休息日"), restDaysWidget);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return QString();

    QList<int> intervals;
    const QStringList parts = intervalEdit->text().split(',', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        bool ok = false;
        int interval = part.trimmed().toInt(&ok);
        if (!ok || interval <= 0) {
            QMessageBox::warning(this, tr("间隔无效"), tr("间隔天数必须是用逗号分隔的正整数。"));
            return QString();
        }
        intervals.append(interval);
    }
    if (intervals.isEmpty()) {
        QMessageBox::warning(this, tr("间隔无效"), tr("请至少输入一个间隔天数。"));
        return QString();
    }

    QList<int> restDays;
    for (QCheckBox *check : restDayChecks) {
        if (check->isChecked())
            restDays.append(check->property("day").toInt());
    }

    return m_reviewManager->createCustomStrategy(tr("固定间隔 %1").arg(intervalEdit->text()), intervals, restDays);
}

void MainWindow::applyReviewStrategy(const QString &noteId, const QString &title, const QString &strategyId)
{
    if (m_reviewManager->hasReviewRecord(noteId))
        m_reviewManager->changeNoteStrategy(noteId, strategyId);
    else
        m_reviewManager->addNoteToReview(noteId, title, strategyId);
}

void MainWindow::onReviewStrategyRequested(const QString &absolutePath, bool fixedInterval)
{
    QFileInfo info(absolutePath);
    if (!info.exists() || !info.isFile())
        return;

    QString strategyId = QStringLiteral("standard_ebbinghaus");
    if (fixedInterval) {
        strategyId = promptFixedIntervalStrategy();
        if (strategyId.isEmpty())
            return;
    }

    applyReviewStrategy(absolutePath, info.completeBaseName(), strategyId);
    refreshReviewTimeline();
    ui->statusbar->showMessage(tr("已加入复习计划: %1").arg(info.completeBaseName()));
}

void MainWindow::onFolderReviewStrategyRequested(const QString &absolutePath, bool fixedInterval)
{
    QFileInfo info(absolutePath);
    if (!info.exists() || !info.isDir())
        return;

    QDirIterator it(absolutePath, {"*.md"}, QDir::Files, QDirIterator::Subdirectories);
    QStringList paths;
    while (it.hasNext())
        paths.append(it.next());

    if (paths.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("所选文件夹没有 Markdown 笔记。"));
        return;
    }

    QString strategyId = QStringLiteral("standard_ebbinghaus");
    if (fixedInterval) {
        strategyId = promptFixedIntervalStrategy();
        if (strategyId.isEmpty())
            return;
    }

    QStringList existingPaths;
    for (const QString &path : paths) {
        if (m_reviewManager->hasReviewRecord(path))
            existingPaths.append(path);
    }

    bool applyToExisting = true;
    if (!existingPaths.isEmpty()) {
        QStringList displayList;
        const int maxDisplay = 8;
        for (int i = 0; i < qMin(existingPaths.size(), maxDisplay); ++i)
            displayList.append(QFileInfo(existingPaths.at(i)).fileName());
        QString body = displayList.join("\n");
        if (existingPaths.size() > maxDisplay)
            body += tr("\n\n以及其他 %1 个文件").arg(existingPaths.size() - maxDisplay);

        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("已有复习计划"));
        box.setText(tr("以下 %1 个文件已经在复习计划中，是否更改它们的复习策略？")
                        .arg(existingPaths.size()));
        box.setInformativeText(body);
        QPushButton *yesButton = box.addButton(tr("是"), QMessageBox::YesRole);
        QPushButton *noButton = box.addButton(tr("否，跳过这些文件"), QMessageBox::NoRole);
        box.addButton(tr("取消"), QMessageBox::RejectRole);
        box.setDefaultButton(yesButton);
        box.exec();

        if (box.clickedButton() == yesButton) {
            applyToExisting = true;
        } else if (box.clickedButton() == noButton) {
            applyToExisting = false;
        } else {
            return;
        }
    }

    int added = 0;
    int updated = 0;
    for (const QString &path : paths) {
        bool hasRecord = m_reviewManager->hasReviewRecord(path);
        if (hasRecord && !applyToExisting)
            continue;
        applyReviewStrategy(path, QFileInfo(path).completeBaseName(), strategyId);
        if (hasRecord)
            updated++;
        else
            added++;
    }

    refreshReviewTimeline();

    if (applyToExisting && updated > 0)
        ui->statusbar->showMessage(tr("已处理 %1 篇笔记，其中 %2 篇已更新复习策略").arg(paths.size()).arg(updated));
    else
        ui->statusbar->showMessage(tr("已将 %1 篇笔记加入复习计划").arg(added));
}

void MainWindow::onReviewTimelineRequested()
{
    ensureReviewTimelineTab();
    refreshReviewTimeline();
    m_fileExplorer->setReviewButtonChecked(true);
    m_tabWidget->setCurrentWidget(m_reviewTimelinePage);
}

void MainWindow::onTimelineNoteDropped(const QString &absolutePath, const QDate &date)
{
    QFileInfo info(absolutePath);
    if (!info.exists() || !info.isFile())
        return;

    QString title = info.completeBaseName();
    m_reviewManager->addManualReviewSchedule(absolutePath, title, QDateTime(date, QTime(9, 0)));
    refreshReviewTimeline();
    ui->statusbar->showMessage(tr("已安排复习: %1").arg(title));
}

void MainWindow::onTimelineManualScheduleDropped(const QString &scheduleId, const QDate &date)
{
    if (scheduleId.isEmpty())
        return;

    m_reviewManager->moveManualSchedule(scheduleId, QDateTime(date, QTime(9, 0)));
    refreshReviewTimeline();
    ui->statusbar->showMessage(tr("已移动手动排期"));
}

void MainWindow::ensureReviewTimelineTab()
{
    if (m_reviewTimelinePage)
        return;

    m_reviewTimelinePage = new QWidget(m_tabWidget);
    QVBoxLayout *timelineLayout = new QVBoxLayout(m_reviewTimelinePage);
    timelineLayout->setContentsMargins(0, 0, 0, 0);
    m_reviewTimeline = new ReviewTimelinePane(m_reviewTimelinePage);
    m_reviewTimeline->setDarkMode(m_themeMode == ThemeMode::Dark);
    timelineLayout->addWidget(m_reviewTimeline);

    int index = m_tabWidget->addTab(m_reviewTimelinePage, tr("复习时间轴"));
    installTabCloseButton(index, m_reviewTimelinePage);
    m_tabWidget->setCurrentIndex(index);

    connect(m_reviewTimeline, &ReviewTimelinePane::noteDropped,
            this, &MainWindow::onTimelineNoteDropped);
    connect(m_reviewTimeline, &ReviewTimelinePane::manualScheduleDropped,
            this, &MainWindow::onTimelineManualScheduleDropped);
    connect(m_reviewTimeline, &ReviewTimelinePane::noteOpenRequested,
            this, &MainWindow::onReviewItemOpenRequested);
    connect(m_reviewTimeline, &ReviewTimelinePane::notePreviewRequested,
            this, &MainWindow::onTimelineNotePreviewRequested);
    connect(m_reviewTimeline, &ReviewTimelinePane::rememberedRequested,
            this, &MainWindow::onTimelineRememberedRequested);
    connect(m_reviewTimeline, &ReviewTimelinePane::forgottenRequested,
            this, &MainWindow::onTimelineForgottenRequested);
    connect(m_reviewTimeline, &ReviewTimelinePane::manualDeleteRequested,
            this, &MainWindow::onTimelineManualDeleteRequested);
    connect(m_reviewTimeline, &ReviewTimelinePane::strategyDateDeleteRequested,
            this, &MainWindow::onTimelineStrategyDateDeleteRequested);
    connect(m_reviewTimeline, &ReviewTimelinePane::strategyAdjustRequested,
            this, &MainWindow::onTimelineStrategyAdjustRequested);
}

void MainWindow::refreshReviewTimeline()
{
    if (!m_reviewTimeline || !m_reviewManager)
        return;

    m_reviewTimeline->setReviewPlan(m_reviewManager->reviewPlanBetween(m_reviewTimeline->startDate(), m_reviewTimeline->endDate()));
}

void MainWindow::onPracticeRequested()
{
    if (m_vaultPath.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请先选择知识库目录。"));
        return;
    }

    PracticeDialog dialog(m_vaultPath, m_themeMode == ThemeMode::Dark, this);
    dialog.exec();

    m_fileExplorer->setPracticeButtonChecked(false);
}

void MainWindow::onGraphRequested()
{
    ensureGraphTab();
    m_tabWidget->setCurrentWidget(m_graphPage);
}

void MainWindow::onAiAssistantRequested()
{
    showAiAssistant(!m_aiAssistantPane->isVisible());
}

void MainWindow::showAiAssistant(bool visible)
{
    m_aiAssistantPane->setVisible(visible);
    if (visible)
        m_splitter->setSizes({220, 780, 440});
    else
        m_splitter->setSizes({220, 1200, 0});
}

bool MainWindow::ensureAiApiKey()
{
    QSettings settings("Qbsidian", "Qbsidian");
    QString key = settings.value("ai/deepseekApiKey").toString();
    if (!key.trimmed().isEmpty()) {
        m_aiAgent->setApiKey(key.trimmed());
        return true;
    }

    return promptAiApiKey(false);
}

bool MainWindow::promptAiApiKey(bool forceUpdate)
{
    bool ok = false;
    QString title = forceUpdate ? tr("重新设置 DeepSeek API Key") : tr("设置 DeepSeek API Key");
    QString input = QInputDialog::getText(this, title,
        tr("请输入 DeepSeek API Key:"), QLineEdit::Password, QString(), &ok);
    input = input.trimmed();
    if (!ok || input.isEmpty())
        return false;

    QSettings settings("Qbsidian", "Qbsidian");
    settings.setValue("ai/deepseekApiKey", input);
    m_aiAgent->setApiKey(input);
    return true;
}

void MainWindow::onAiSettingsRequested()
{
    showAiAssistant(true);
    if (promptAiApiKey(true))
        m_aiAssistantPane->appendSystemMessage(tr("API Key 已更新"));
}

QString MainWindow::normalizeSelectedText(QString text) const
{
    text.replace(QChar::ParagraphSeparator, '\n');
    text.replace(QChar::LineSeparator, '\n');
    return text.trimmed();
}

QString MainWindow::selectedNoteText() const
{
    NoteTab *tab = currentTab();
    if (!tab)
        return QString();

    QString selected = normalizeSelectedText(tab->editor->textCursor().selectedText());
    if (!selected.isEmpty())
        return selected;

    return normalizeSelectedText(tab->preview->textCursor().selectedText());
}

void MainWindow::onAiQuestionSubmitted(const QString &text)
{
    showAiAssistant(true);
    if (!ensureAiApiKey())
        return;

    QString selected = selectedNoteText();
    bool quoted = !selected.isEmpty();
    m_aiAssistantPane->appendUserMessage(text, quoted);
    m_aiPendingAction = AiPendingAction::Chat;

    if (quoted) {
        QString prompt = tr("以下是我选中的笔记内容：\n%1\n\n我的问题是：\n%2").arg(selected, text);
        m_aiAgent->askQuestion(prompt);
    } else {
        m_aiAgent->askQuestion(text);
    }
}

void MainWindow::onAiSummaryRequested()
{
    showAiAssistant(true);
    QString selected = selectedNoteText();
    if (selected.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请先在编辑区或预览区选中要总结的内容。"));
        return;
    }
    if (!ensureAiApiKey())
        return;

    m_aiAssistantPane->appendUserMessage(tr("总结我选中的内容"), true);
    m_aiPendingAction = AiPendingAction::Summary;
    m_aiAgent->summarizeNote(selected);
}

void MainWindow::onAiQuizRequested()
{
    showAiAssistant(true);
    if (!currentTab()) {
        QMessageBox::information(this, tr("提示"), tr("请先打开一篇笔记。"));
        return;
    }

    QString selected = selectedNoteText();
    if (selected.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请先在编辑区或预览区选中要出题的内容。"));
        return;
    }
    if (!ensureAiApiKey())
        return;

    m_aiAssistantPane->appendUserMessage(tr("根据选中内容给我出题"), true);
    m_aiPendingAction = AiPendingAction::Quiz;
    m_aiAgent->generateQuiz(selected);
}

void MainWindow::onAiResponseReceived(const QString &replyText)
{
    m_aiAssistantPane->setBusy(false);

    if (m_aiPendingAction == AiPendingAction::Quiz) {
        appendQuizToCurrentNote(replyText);
        m_aiAssistantPane->appendSystemMessage(tr("已将题目加入当前笔记末尾"));
    } else {
        m_aiAssistantPane->appendAiMessage(replyText);
    }

    m_aiPendingAction = AiPendingAction::None;
}

void MainWindow::onAiErrorOccurred(const QString &errorMsg)
{
    m_aiAssistantPane->setBusy(false);
    m_aiPendingAction = AiPendingAction::None;
    QMessageBox::warning(this, tr("AI 助手出错了"), errorMsg);
}

void MainWindow::appendQuizToCurrentNote(const QString &quizText)
{
    NoteTab *tab = currentTab();
    if (!tab)
        return;

    QStringList quizLines;
    const QStringList lines = quizText.split('\n');
    for (QString line : lines) {
        line = line.trimmed();
        if (line.startsWith(QStringLiteral("#题库/")) && line.contains(QStringLiteral("::")))
            quizLines.append(line);
    }

    QString contentToAppend = quizLines.isEmpty() ? quizText.trimmed() : quizLines.join('\n');
    if (contentToAppend.isEmpty())
        return;

    QString content = tab->editor->toPlainText();
    if (!content.endsWith('\n'))
        content += '\n';
    content += '\n' + contentToAppend + '\n';
    tab->editor->setPlainText(content);
    QTextCursor cursor = tab->editor->textCursor();
    cursor.movePosition(QTextCursor::End);
    tab->editor->setTextCursor(cursor);
    tab->preview->showHtml(content);
    tab->isModified = true;
    updateTabTitle(tab);
    updateTitle();
    ui->statusbar->showMessage(tr("已将题目加入当前笔记末尾"));
}

void MainWindow::ensureGraphTab()
{
    if (m_graphPage)
        return;

    m_graphPage = new QWidget(m_tabWidget);
    QVBoxLayout *graphLayout = new QVBoxLayout(m_graphPage);
    graphLayout->setContentsMargins(0, 0, 0, 0);

    m_graphPane = new GraphPane(m_graphPage);
    m_graphPane->setDarkMode(m_themeMode == ThemeMode::Dark);
    graphLayout->addWidget(m_graphPane);

    int index = m_tabWidget->addTab(m_graphPage, tr("知识图谱"));
    installTabCloseButton(index, m_graphPage);
    m_tabWidget->setCurrentIndex(index);

    connect(m_graphPane, &GraphPane::noteOpenRequested, this, [this](const QString &filePath) {
        openFileInTab(filePath);
    });

    m_graphPane->loadVault(m_vaultPath);
}

void MainWindow::onRenameRequested(const QString &absolutePath, const QString &newName)
{
    if (newName.isEmpty())
        return;

    QFileInfo oldInfo(absolutePath);
    bool oldIsDir = oldInfo.isDir();

    QString currentName = oldIsDir ? oldInfo.fileName() : oldInfo.completeBaseName();
    if (newName == currentName)
        return;

    if (newName.contains('/') || newName.contains('\\')) {
        QMessageBox::warning(this, tr("非法名称"), tr("文件名不能包含路径分隔符。"));
        return;
    }

    QString dir = oldInfo.absoluteDir().absolutePath();
    QString newPath;
    if (!oldIsDir && oldInfo.suffix() == "md")
        newPath = dir + "/" + newName + ".md";
    else
        newPath = dir + "/" + newName;

    m_noteManagerHadError = false;
    if (!m_noteManager->renameItem(absolutePath, newPath))
        return;

    if (oldIsDir) {
        QStringList keysNeedingUpdate;
        for (NoteTab *tab : m_tabsByPath) {
            if (tab->filePath == absolutePath || tab->filePath.startsWith(absolutePath + "/"))
                keysNeedingUpdate.append(tab->filePath);
        }

        for (const QString &oldKey : keysNeedingUpdate) {
            NoteTab *tab = m_tabsByPath.take(oldKey);
            QString relative = oldKey.mid(absolutePath.length());
            tab->filePath = newPath + relative;
            m_tabsByPath.insert(tab->filePath, tab);
            updateTabTitle(tab);
        }

        m_reviewManager->renameNoteRecordsPrefix(absolutePath, newPath);
    } else {
        if (m_tabsByPath.contains(absolutePath)) {
            NoteTab *tab = m_tabsByPath.take(absolutePath);
            tab->filePath = newPath;
            m_tabsByPath.insert(newPath, tab);
            updateTabTitle(tab);
        }

        m_reviewManager->renameNoteRecord(absolutePath, newPath);
    }

    m_fileExplorer->setRootPath(m_vaultPath);
    refreshReviewTimeline();

    if (m_graphPane)
        m_graphPane->loadVault(m_vaultPath);

    QFileInfo newInfo(newPath);
    if (!oldIsDir && newInfo.suffix() == "md")
        openFileInTab(newPath);

    ui->statusbar->showMessage(tr("已重命名: %1").arg(newInfo.fileName()));
}
