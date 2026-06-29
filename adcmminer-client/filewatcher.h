#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QThread>

class FileWatcher : public QObject
{
    Q_OBJECT
public:
    FileWatcher(const QString &path, QObject *parent = nullptr);

signals:
    void onFileChanged(const QString &);
public slots:
    void operate();
    void operateNewPath(const QString &);
private slots:
    void fileChanged(const QString &);
private:
    QString m_path;
    QFileInfo *m_fileInfo;
};

#endif // FILEWATCHER_H
