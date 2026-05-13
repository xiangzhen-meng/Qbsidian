#ifndef EDITORPANE_H
#define EDITORPANE_H

#include <QPlainTextEdit>
#include <QColor>

class LineNumberArea;

class EditorPane : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit EditorPane(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setThemeColors(const QColor &lineBg, const QColor &lineFg);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *m_lineNumberArea;
    QColor m_lineBg;
    QColor m_lineFg;
};

#endif // EDITORPANE_H
