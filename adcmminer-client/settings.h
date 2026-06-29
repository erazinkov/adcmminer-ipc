#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QStringList>

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings(QString fileName = "settings.ini", QObject *parent = nullptr);

    void writeSettings();

    const QString &path() const;
    void setPath(const QString &newPath);

private:
    QString m_fileName;
    QString m_path;

    void readSettings();

};

#endif // SETTINGS_H
