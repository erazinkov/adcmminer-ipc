#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName(QString("SAI-Miner"));
    a.setApplicationVersion(QString("1.0.0"));

    MainWindow w;
    QString title{QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion()};
    w.setWindowTitle(title);

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        QRect screenGeometry = primaryScreen->availableGeometry();
        w.resize(static_cast<int>(0.8 * screenGeometry.width()), static_cast<int>(0.8 * screenGeometry.height()));
    }
    w.show();

    return a.exec();
}
