#include <QCoreApplication>

#include <QTextStream>
#include "server.h"

const QString RESET ="\033[0m";
const QString RED = "\033[31m";
const QString GREEN ="\033[32m";
const QString BOLD = "\033[1m";
const QString YELLOW = "\033[33m";

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("ADCMMiner Server");
    a.setApplicationVersion("1.0.0");

    const QString serverName = "ADCMMiner Server1";
    Server server;

    QObject::connect(&server, &Server::serverStarted, [&serverName]() {
        QTextStream out(stdout);
        out << serverName << " - " << GREEN << BOLD << "Succeed " << RESET << Qt::endl;
    });
    QObject::connect(&server, &Server::serverError, [&serverName](const QString &errorText) {
        QTextStream out(stdout);
        out << serverName << " - " << RED << BOLD << "Failed " << RESET << errorText << Qt::endl;
    });

    if (!server.start(serverName)) {
        return 1;
    }

    QTimer statusTimer;
    QObject::connect(&statusTimer, &QTimer::timeout, [&server, serverName]() {
        QDateTime now{QDateTime::currentDateTime()};
        QTextStream out(stdout);
        out << serverName << " - ";
        out << (server.isRunning() ? GREEN : RED) << BOLD << now.toString("yyyy-MM-dd hh:mm:ss") << RESET;
        out << Qt::endl;

    });
    statusTimer.start(30'000);

    return a.exec();
}
