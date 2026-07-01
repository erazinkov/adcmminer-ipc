#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QTimer>

#include "protocol.h"

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);
    ~Client();

    void connectToServer(const QString &serverName);
    void disconnectFromServer();
    void sendTask(const TaskData &task);

signals:
    void connected();
    void disconnected();
    void connectionError(const QString &errorText);
    void resultReceived(ResultData resultData);
    void complexDataReceived(const QMap<QString, QList<QPointF>> &data, const QMap<QString, QStringList> &text);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QLocalSocket::LocalSocketError socketError);

    void attemptReconnect();   // Слот для отправки запроса на подключение
    void handleHeartbeatTimeout(); // Слот, если сервер долго молчит

    void sendHeartbeat();
private:
    QLocalSocket *m_socket;
    QTimer *m_timer;
    QString m_serverName;

    QTimer *m_heartbeatTimer;
    QTimer *m_reconnectTimer;

    const int HEARTBEAT_INTERVAL_MS = 3'000;
    const int HEARTBEAT_TIMEOUT_MS = 10'000;
    const int RECONNECT_INTERVAL_MS = 3'000;
};

#endif // CLIENT_H
