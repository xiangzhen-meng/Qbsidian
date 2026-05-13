#ifndef PREVIEWPANE_H
#define PREVIEWPANE_H

#include <QTextBrowser>

class PreviewPane : public QTextBrowser
{
    Q_OBJECT

public:
    explicit PreviewPane(QWidget *parent = nullptr);

    void setDarkMode(bool dark);
    void showHtml(const QString &markdown);

private:
    QString buildStyleSheet() const;

    bool m_darkMode;
};

#endif // PREVIEWPANE_H
