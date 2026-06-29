#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMap>
#include <QJsonObject>
#include <QTimer>
#include <QThreadPool>
#include "protocol.h"

#include "controller.h"

struct ClientConnection {
    QLocalSocket *socket = nullptr;
    QString identifier;
    quintptr id;
    QByteArray buffer;
};

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    bool start(const QString &serverName);
    void stop();
    bool isRunning() const;
    QString errorString() const;

    // Server statistics
    int activeConnections() const;
    QString serverName() const;

signals:
    void serverStarted();
    void serverStopped();
    void clientConnected(quintptr clientId);
    void clientDisconnected(quintptr clientId);

    void errorOccurred(const QString &error);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void processFrames(ClientConnection *client);
    void handleGetStatus(ClientConnection *client, const QByteArray &payload);
    void handleHeartbeat(ClientConnection *client);
    void handleShutdown(ClientConnection *client);

    void sendFrame(QLocalSocket *socket, Protocol::MessageType type,
                  const QString &requestId, const QByteArray &payload);
    void sendError(QLocalSocket *socket, const QString &requestId,
                  const QString &error);
    void sendProgress(QLocalSocket *socket, const QString &requestId,
                     int percentage, const QString &message);
    void sendResponse(QLocalSocket *socket, const QString &requestId,
                     const QJsonValue &result);

    QLocalServer *m_server;
    QMap<quintptr, ClientConnection *> m_clients;
    quintptr m_nextClientId;
    QThreadPool *m_threadPool;

    Controller *m_controller;
};


#endif // SERVER_H

