#include <QCoreApplication>

#include <QTextStream>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("ADCMMiner Server");
    a.setApplicationVersion("1.0.0");

    const QString serverName = "ADCMMiner Server";
    Server server;

     // Connect signals for monitoring
     QObject::connect(&server, &Server::serverStarted, []() {
         QTextStream(stdout) << "✓ Server is ready\n";
     });

     QObject::connect(&server, &Server::clientConnected,
         [](quintptr clientId) {
         QTextStream(stdout) << "✓ Client " << clientId << " connected\n";
     });

     QObject::connect(&server, &Server::clientDisconnected,
         [](quintptr clientId) {
         QTextStream(stdout) << "✗ Client " << clientId << " disconnected\n";
     });

     QObject::connect(&server, &Server::errorOccurred,
         [](const QString &error) {
         QTextStream(stderr) << "✗ Error: " << error << "\n";
     });

     if (!server.start(serverName)) {
         QTextStream(stderr) << "Failed to start server: " << server.errorString() << "\n";
         return 1;
     }

     // Show periodic status
     QTimer statusTimer;
     QObject::connect(&statusTimer, &QTimer::timeout, [&server]() {
         if (server.isRunning()) {
             QTextStream(stdout) << "════════ Status ════════\n";
             QTextStream(stdout) << "  Connections: " << server.activeConnections() << "\n";
             QTextStream(stdout) << "═══════════════════════\n";
         }
     });
     statusTimer.start(30000); // Every 30 seconds

    return a.exec();
}
