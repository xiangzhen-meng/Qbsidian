#include "editorpane.h"
#include "autohidescrollareafilter.h"

EditorPane::EditorPane(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 2);
    new AutoHideScrollAreaFilter(this, this);
}
