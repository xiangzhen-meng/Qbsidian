#ifndef PREVIEWPANE_H
#define PREVIEWPANE_H

#include <QTextBrowser>

class PreviewPane : public QTextBrowser
{
    Q_OBJECT

public:
    explicit PreviewPane(QWidget *parent = nullptr);

    void showHtml(const QString &markdown);
};

#endif // PREVIEWPANE_H
