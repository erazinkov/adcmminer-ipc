#include "filewatcher.h"

#include <QDebug>

FileWatcher::FileWatcher(const QString &path, QObject *parent) : QObject(parent), m_path(path)
{
    m_fileInfo = new QFileInfo(m_path);
}

void FileWatcher::operate()
{
    auto lm{m_fileInfo->lastModified()};
    m_fileInfo->refresh();
    if (lm != m_fileInfo->lastModified())
    {
        const QString pathAndTime{QString("%1 - %2").arg(m_fileInfo->path()).arg(m_fileInfo->lastModified().toString())};
        emit(onFileChanged(pathAndTime));
    }
}

void FileWatcher::operateNewPath(const QString &newPath)
{
    m_fileInfo->setFile(newPath);
}

void FileWatcher::fileChanged(const QString &path)
{
    auto lm{m_fileInfo->lastModified()};
    auto bt{m_fileInfo->birthTime()};
    qInfo() << lm << bt;
    m_fileInfo->refresh();
    if (lm != m_fileInfo->lastModified())
    {
        const QString pathAndTime{QString("%1 - %2").arg(path).arg(m_fileInfo->lastModified().toString())};
        emit(onFileChanged(pathAndTime));
    }
}
