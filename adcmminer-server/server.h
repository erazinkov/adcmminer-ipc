#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMap>
#include <QTimer>

#include "controller.h"
#include "protocol.h"

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);

    bool start(const QString &serverName);
    bool isRunning() const;
signals:
    void serverStarted();
    void serverError(const QString &errorText);
private slots:
    void onNewConnection();
    void processIncomingData(QLocalSocket *clientSocket);
    void onClientDisconnected();

private:
    QLocalServer *m_server;
    Controller *m_controller;

    void sendResultToClient(QLocalSocket *clientSocket, const ResultData &result);
};


#endif // SERVER_H

