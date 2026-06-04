#include "aiagent.h"
#include <QNetworkRequest>
#include <QUrl>

AIAgent::AIAgent(QObject *parent)
    : QObject(parent),
    m_networkManager(new QNetworkAccessManager(this))
{
    // 默认使用 DeepSeek 的 API 地址和模型。
    m_apiUrl = "https://api.deepseek.com/chat/completions";
    m_modelName = "deepseek-v4-pro";
}

void AIAgent::setApiKey(const QString &key)
{
    m_apiKey = key;
}


void AIAgent::askQuestion(const QString &userInput)
{
    QString systemPrompt = "你是一个名为 Qbsidian 的知识管理助手，请用简明扼要的中文回答用户的问题。";
    sendPostRequest(systemPrompt, userInput);
}

void AIAgent::summarizeNote(const QString &selectedText)
{
    QString systemPrompt = "你是一个专业的学习助理。请将用户发来的笔记内容进行提炼总结，输出3-5条核心要点（使用 Markdown 列表格式）。";
    sendPostRequest(systemPrompt, selectedText);
}

void AIAgent::generateQuiz(const QString &selectedText)
{
    QString systemPrompt = "你是一个严厉的老师。请根据用户发来的知识点，生成 3 道测试题（单选或问答形式均可），并附带标准答案解析。";
    sendPostRequest(systemPrompt, selectedText);
}


void AIAgent::sendPostRequest(const QString &systemPrompt, const QString &userPrompt)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API Key 尚未配置，请先设置 API Key！");
        return;
    }

    emit requestStarted();

    QJsonObject rootObj;
    rootObj["model"] = m_modelName;

    QJsonArray messagesArray;

    // 系统提示词（人设）
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = systemPrompt;
    messagesArray.append(systemMsg);

    // 用户实际输入的内容
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userPrompt;
    messagesArray.append(userMsg);

    rootObj["messages"] = messagesArray;
    rootObj["temperature"] = 0.8;

    QJsonDocument doc(rootObj);
    QByteArray jsonData = doc.toJson();

    QNetworkRequest request;
    request.setUrl(QUrl(m_apiUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString authHeader = "Bearer " + m_apiKey;
    request.setRawHeader("Authorization", authHeader.toUtf8());

    QNetworkReply *reply = m_networkManager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {

        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            // 处理网络错误 (如断网、API错误等)
            emit errorOccurred("网络请求失败: " + reply->errorString());
            return;
        }
        QByteArray responseData = reply->readAll();
        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &jsonError);

        if (jsonError.error != QJsonParseError::NoError) {
            emit errorOccurred("JSON 解析失败！");
            return;
        }

        QJsonObject resObj = jsonDoc.object();
        if (resObj.contains("choices") && resObj["choices"].isArray()) {
            QJsonArray choices = resObj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices[0].toObject();
                if (firstChoice.contains("message")) {
                    QJsonObject message = firstChoice["message"].toObject();
                    QString answer = message["content"].toString();

                    emit responseReceived(answer);
                    return;
                }
            }
        }

        emit errorOccurred("未获取到有效的 AI 回复内容。");
    });
}
