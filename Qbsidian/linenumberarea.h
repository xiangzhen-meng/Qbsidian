#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

class EditorPane;

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(EditorPane *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    EditorPane *m_editor;
};

#endif // LINENUMBERAREA_H
