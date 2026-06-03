#ifndef PRACTICEDIALOG_H
#define PRACTICEDIALOG_H

#include <QDialog>
#include <QList>
#include "questiondata.h"

class QTextBrowser;
class QPushButton;
class QLabel;

class PracticeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PracticeDialog(const QList<ExtractedQuestion> &questions, bool darkMode, QWidget *parent = nullptr);

private slots:
    void onShowAnswer();
    void onKnown();
    void onUnknown();

private:
    void showCurrentQuestion();
    void advanceQuestion();
    QString buildStyleSheet() const;
    QString buildContentStyleSheet() const;
    QString renderQuestionHtml(const ExtractedQuestion &question, bool showAnswer) const;

    QList<ExtractedQuestion> m_questions;
    int m_currentIndex;
    bool m_answerVisible;
    bool m_darkMode;

    QTextBrowser *m_browser;
    QPushButton *m_showAnswerButton;
    QPushButton *m_knownButton;
    QPushButton *m_unknownButton;
    QLabel *m_progressLabel;
    QLabel *m_sourceLabel;
    QLabel *m_tagLabel;
};

#endif // PRACTICEDIALOG_H
