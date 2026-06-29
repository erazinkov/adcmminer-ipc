#include "filewatcherworker.h"

FileWatcherWorker::FileWatcherWorker(const QString &path, QObject *parent)
    : QObject{parent}
{
    m_fileInfo = new QFileInfo(path);
}

void FileWatcherWorker::doWorkCheck()
{
    QString r;
    bool isModified{false};
    if (m_fileInfo->isFile() && m_fileInfo->exists()) {
        r.append(QString("<span style='color: green;'>%1</span> %2 %3").arg(QChar(0x2714)).arg(m_fileInfo->absoluteFilePath()).arg(m_fileInfo->lastModified().toString()));
        const auto p{m_fileInfo->lastModified()};
        m_fileInfo->refresh();
        const auto n{m_fileInfo->lastModified()};
        isModified = p != n;
        r.append(isModified ? QString::fromUtf8(u8"\U0001F525") : QString::fromUtf8(u8"\U000023F3"));
    } else {
        r.append(QString("<span style='color: red;'>%1</span> %2").arg(QChar(0x2718)).arg(m_fileInfo->absoluteFilePath()));
    }
    QString p{m_fileInfo->absoluteFilePath()};
    emit resultReadyCheck(r, p, isModified);
}

void FileWatcherWorker::doWorkPath(const QString &newPath)
{
    m_fileInfo->setFile(newPath);
}
