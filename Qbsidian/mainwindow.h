#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QString>

class FileExplorerPane;
class EditorPane;
class PreviewPane;
class NoteManager;
class QSplitter;
class QTabWidget;
class QTimer;
class QWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class ThemeMode { Light, Dark };

private:
    struct NoteTab
    {
        QWidget *page;
        QSplitter *splitter;
        EditorPane *editor;
        PreviewPane *preview;
        QTimer *previewTimer;
        QString filePath;
        bool isModified;
    };

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFileSelected(const QString &absoluteFilePath);
    void onSave();
    void onNewNoteRequested(const QString &directory, const QString &baseName);
    void onNewFolderRequested(const QString &directory, const QString &folderName);
    void toggleTheme();
    void onNoteManagerError(const QString &operation, const QString &reason);
    void onDeleteRequested(const QString &absolutePath);
    void onAnchorClicked(const QUrl &url);
    void onSearchResultClicked(const QString &filePath, int lineNumber);
    void onTabCloseRequested(int index);
    void onCurrentTabChanged(int index);

private:
    void setupUi();
    void applyWindowTheme();
    void applyContentTheme();
    void setupMenuBar();
    void setupPanes();
    void connectSignals();
    QString promptVaultDirectory();
    void updateTitle();
    bool openFileInTab(const QString &path);
    bool saveCurrentTab();
    bool saveTab(NoteTab *tab);
    void updateTabTitle(NoteTab *tab);
    NoteTab *currentTab() const;
    void closeTabAt(int index);
    void jumpCurrentTabToLine(int lineNumber);
    void loadThemeSetting();
    void saveThemeSetting();
    QString buildLightQss() const;
    QString buildDarkQss() const;

    Ui::MainWindow *ui;
    QSplitter *m_splitter;
    FileExplorerPane *m_fileExplorer;
    QTabWidget *m_tabWidget;
    NoteManager *m_noteManager;

    QHash<QString, NoteTab *> m_tabsByPath;
    QString m_vaultPath;
    bool m_noteManagerHadError;
    ThemeMode m_themeMode;
};

#endif // MAINWINDOW_H
