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

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QLocalSocket::LocalSocketError socketError);

    void sendHeartbeat();
private:
    QLocalSocket *m_socket;
    QTimer *m_timer;
    QString m_serverName;

    QTimer *m_heartbeatTimer;
};

#endif // CLIENT_H
