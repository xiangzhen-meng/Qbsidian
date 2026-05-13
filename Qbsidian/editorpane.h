#ifndef EDITORPANE_H
#define EDITORPANE_H

#include <QPlainTextEdit>

class EditorPane : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit EditorPane(QWidget *parent = nullptr);
};

#endif // EDITORPANE_H
