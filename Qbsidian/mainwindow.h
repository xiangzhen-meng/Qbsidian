#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class FileExplorerPane;
class EditorPane;
class PreviewPane;
class QSplitter;
class QTimer;

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

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFileSelected(const QString &absoluteFilePath);
    void onTextChanged();
    void onPreviewTimer();
    void onSave();
    void onNewNoteRequested(const QString &directory, const QString &baseName);
    void onNewFolderRequested(const QString &directory, const QString &folderName);
    void toggleTheme();

private:
    void setupUi();
    void applyWindowTheme();
    void applyContentTheme();
    void setupMenuBar();
    void setupPanes();
    void connectSignals();
    QString promptVaultDirectory();
    void updateTitle();
    void loadFile(const QString &path);
    void saveFile();
    void loadThemeSetting();
    void saveThemeSetting();
    QString buildLightQss() const;
    QString buildDarkQss() const;

    Ui::MainWindow *ui;
    QSplitter *m_splitter;
    FileExplorerPane *m_fileExplorer;
    EditorPane *m_editor;
    PreviewPane *m_preview;
    QTimer *m_previewTimer;

    QString m_currentFilePath;
    QString m_vaultPath;
    bool m_isModified;
    ThemeMode m_themeMode;
};

#endif // MAINWINDOW_H
