#ifndef NOTEMANAGER_H
#define NOTEMANAGER_H

#include <QObject>
#include <QString>
// notemanager.h
struct SearchResult
{
    QString filePath;       // 文件绝对路径
    int lineNumber;         // 匹配到的行号
    QString lineContent;    // 匹配到的那一行的完整内容
};
;
class NoteManager : public QObject
{
    Q_OBJECT
public:
    explicit NoteManager(QObject *parent = nullptr);


    QString load(const QString &absoluteFilePath);
    bool save(const QString &absoluteFilePath, const QString &content);
    QString createNewNote(const QString &directory, const QString &baseName);
    QString createNewFolder(const QString &directory, const QString &folderName);
    bool exists(const QString &absoluteFilePath) const;
    bool deleteItem(const QString &absolutePath);
    QString findNotePath(const QString &directory, const QString &noteName) const;

    QVector<SearchResult> searchInVault(const QString &directory, const QRegularExpression &regex) const;

signals:
    void errorOccurred(const QString &operation, const QString &reason);
};

#endif // NOTEMANAGER_H
