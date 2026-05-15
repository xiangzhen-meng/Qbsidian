#include "notemanager.h"
#include <QSaveFile>
#include <QDir>
#include <QFileInfo>

NoteManager::NoteManager(QObject *parent) : QObject(parent){}
//存文件
bool NoteManager::save(const QString &absoluteFilePath, const QString &content)
{
    // 1. 确保父目录存在
    QFileInfo fileInfo(absoluteFilePath);
    QDir().mkpath(fileInfo.absolutePath());

    // 2. 使用 QSaveFile 写入
    QSaveFile file(absoluteFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        emit errorOccurred("保存文件", "无法打开文件进行写入: " + file.errorString());
        return false;
    }

    // 3. 转换UTF-8 编码写入
    QByteArray data = content.toUtf8();


    if (file.write(data) != data.size())
    {
        emit errorOccurred("保存文件", "写入数据不完整，可能磁盘空间不足");
        return false;
    }
    // 4. 提交更改
    if (!file.commit())
    {
        emit errorOccurred("保存文件", "提交保存失败: " + file.errorString());
        return false;
    }
    return true;
}

//读取文件
QString NoteManager::load(const QString &absoluteFilePath)
{
    QFile file(absoluteFilePath);

    // 1. 存在性检查
    if (!file.exists())
    {
        emit errorOccurred("读取文件", "文件不存在: " + absoluteFilePath);
        return QString();
    }

    // 2. 打开文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit errorOccurred("读取文件", "无法打开文件: " + file.errorString());
        return QString();
    }

    // 3. 以 UTF-8 读取
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QString content = in.readAll();
    file.close();
    return content;
}

//创建笔记
QString NoteManager::createNewNote(const QString &directory, const QString &baseName)
{
    QDir dir(directory);
    QString fileName = baseName + ".md";
    QString filePath = dir.absoluteFilePath(fileName);

    int counter = 1;
    while (QFileInfo::exists(filePath)) {
        fileName = QString("%1(%2).md").arg(baseName).arg(counter);
        filePath = dir.absoluteFilePath(fileName);
        counter++;
    }

    if (save(filePath, ""))
    {
        return filePath; // 返回最终的绝对路径，可用于自动打开这个文件
    }
    return QString();
}

//创建文件夹
QString NoteManager::createNewFolder(const QString &directory, const QString &folderName)
{
    QDir dir(directory);
    QString targetPath = dir.absoluteFilePath(folderName);

    if (QDir(targetPath).exists())
    {
        emit errorOccurred("创建文件夹", "文件夹已存在: " + targetPath);
        return QString();
    }

    if (!dir.mkpath(folderName))
    {
        emit errorOccurred("创建文件夹", "创建失败: " + targetPath);
        return QString();
    }

    return targetPath;
}

//状态查询
bool NoteManager::exists(const QString &absoluteFilePath) const
{
    return QFileInfo::exists(absoluteFilePath);
}
