#ifndef AIAGENT_H
#define AIAGENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class AIAgent : public QObject
{
    Q_OBJECT

public:
    explicit AIAgent(QObject *parent = nullptr);

    void setApiKey(const QString &key);

public slots:
    void askQuestion(const QString &userInput);
    void summarizeNote(const QString &selectedText);
    void generateQuiz(const QString &selectedText);

signals:
    void requestStarted();
    void responseReceived(const QString &replyText);
    void errorOccurred(const QString &errorMsg);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiKey;
    QString m_apiUrl;
    QString m_modelName;

    void sendPostRequest(const QString &systemPrompt, const QString &userPrompt);
};

#endif // AIAGENT_H
