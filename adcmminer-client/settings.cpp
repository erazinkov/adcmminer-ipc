#include "settings.h"

#include <QSettings>

Settings::Settings(QString fileName, QObject *parent)
    : QObject{parent}, m_fileName{fileName}
{
    readSettings();
}

void Settings::readSettings()
{
    QSettings settings(m_fileName, QSettings::IniFormat);
    settings.beginGroup("Main");
    {
        auto path = settings.value("path").value<QString>();
        if (!path.isEmpty()) {
            setPath(path);
        } else {
            setPath("/misc/agpk_std/adcm.dat");
        }
    }
    settings.endGroup();
}

void Settings::writeSettings()
{
    QSettings settings(m_fileName, QSettings::IniFormat);

    QString path{m_path};

    settings.beginGroup("Main");
    settings.setValue("path", QVariant::fromValue(path));

    settings.endGroup();
}

const QString &Settings::path() const
{
    return m_path;
}

void Settings::setPath(const QString &newPath)
{
    m_path = newPath;
}
