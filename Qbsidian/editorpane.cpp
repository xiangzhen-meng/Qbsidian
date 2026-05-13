#include "editorpane.h"
#include "linenumberarea.h"
#include <QPainter>
#include <QTextBlock>

EditorPane::EditorPane(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineBg(QColor("#f7f6f3"))
    , m_lineFg(QColor("#c0bfbc"))
{
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 2);
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &EditorPane::blockCountChanged,
            this, &EditorPane::updateLineNumberAreaWidth);
    connect(this, &EditorPane::updateRequest,
            this, &EditorPane::updateLineNumberArea);

    updateLineNumberAreaWidth(0);
}

void EditorPane::setThemeColors(const QColor &lineBg, const QColor &lineFg)
{
    m_lineBg = lineBg;
    m_lineFg = lineFg;
    m_lineNumberArea->update();
}

int EditorPane::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void EditorPane::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void EditorPane::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void EditorPane::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void EditorPane::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), m_lineBg);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(m_lineFg);
            painter.drawText(0, top, m_lineNumberArea->width() - 3,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
