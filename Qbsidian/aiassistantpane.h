#ifndef AIASSISTANTPANE_H
#define AIASSISTANTPANE_H

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QResizeEvent;
class QScrollArea;
class QVBoxLayout;

enum class AiMessageRole
{
    Ai,
    User,
    System
};

class AIAssistantPane : public QWidget
{
    Q_OBJECT

public:
    explicit AIAssistantPane(QWidget *parent = nullptr);

    void appendMessage(AiMessageRole role, const QString &text, bool quoted = false);
    void appendUserMessage(const QString &text, bool quoted = false);
    void appendAiMessage(const QString &text);
    void appendSystemMessage(const QString &text);
    void setBusy(bool busy);
    void setDarkMode(bool dark);

signals:
    void closeRequested();
    void questionSubmitted(const QString &text);
    void settingsRequested();
    void summarizeRequested();
    void quizRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QWidget *createAvatar(AiMessageRole role);
    QWidget *createBubble(AiMessageRole role, const QString &text, bool quoted);
    void scrollToBottom();
    void updateBubbleWidths();
    void updateTheme();

    QScrollArea *m_scrollArea;
    QWidget *m_messageContainer;
    QVBoxLayout *m_messageLayout;
    QPlainTextEdit *m_inputEdit;
    QPushButton *m_sendButton;
    QPushButton *m_summaryButton;
    QPushButton *m_quizButton;
    QPushButton *m_settingsButton;
    QPushButton *m_closeButton;
    QLabel *m_titleLabel;
    QList<QLabel *> m_bubbles;
    bool m_darkMode;
};

#endif // AIASSISTANTPANE_H
