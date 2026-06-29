#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QJsonObject>
#include <QTimer>
#include "protocol.h"

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(const QString &serverName, QObject *parent = nullptr);
    ~Client();

    bool connectToServer(int timeoutMs = 5'000);
    void disconnectFromServer();
    bool isConnected() const;

    void requestServerStatus();
    void sendHeartbeat();

signals:
    void connected();
    void disconnected();
    void connectionError(const QString &error);
    void requestError(const QString &requestId, const QString &error);

    void serverStatusReceived(const QJsonObject &status);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QLocalSocket::LocalSocketError socketError);

private:
    void processFrame();
    void handleProgress(const QByteArray &payload);
    void handleError(const QByteArray &payload);
    void handleStatusResponse(const QByteArray &payload);

    QString sendRequest(const QString &request,
                                const QJsonObject &params);
    void sendFrame(Protocol::MessageType type, const QString &requestId,
                  const QByteArray &payload);

    QLocalSocket *m_socket;
    QString m_serverName;
    QByteArray m_buffer;
    QMap<QString, QString> m_pendingRequests;
    QTimer *m_timer;

};

#endif // CLIENT_H
