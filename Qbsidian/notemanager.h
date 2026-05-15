#ifndef NOTEMANAGER_H
#define NOTEMANAGER_H

#include <QObject>
#include <QString>

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

signals:
    void errorOccurred(const QString &operation, const QString &reason);
};

#endif // NOTEMANAGER_H
