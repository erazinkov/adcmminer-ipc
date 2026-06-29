#include "client.h"

#include <QDataStream>
#include <QJsonDocument>
#include <QTextStream>
#include <iostream>

Client::Client(const QString &serverName, QObject *parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
    , m_serverName(serverName)
{
    connect(m_socket, &QLocalSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &Client::onDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &Client::onReadyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &Client::onError);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);

    connect(m_timer, &QTimer::timeout, this, [&](){
        const QString serverName = "ADCMMiner Server";
        connectToServer();
    });

    connectToServer();
}

Client::~Client()
{
    disconnectFromServer();
}

bool Client::connectToServer(int timeoutMs)
{
    if (m_socket->state() == QLocalSocket::ConnectedState) {
        return true;
    }

    m_socket->connectToServer(m_serverName);

    if (!m_socket->waitForConnected(timeoutMs)) {
        emit connectionError(m_socket->errorString());
        m_timer->start(1'000);
        return false;
    }

    return true;
}

void Client::disconnectFromServer()
{
    if (m_socket->state() == QLocalSocket::ConnectedState) {
        m_socket->disconnectFromServer();
        if (m_socket->state() != QLocalSocket::UnconnectedState) {
            m_socket->waitForDisconnected(5'000);
        }
    }
}

bool Client::isConnected() const
{
    return m_socket->state() == QLocalSocket::ConnectedState;
}

void Client::requestServerStatus()
{
    sendFrame(Protocol::MessageType::GetStatus, "", QByteArray());
}

void Client::sendHeartbeat()
{
    sendFrame(Protocol::MessageType::Heartbeat, "", QByteArray());
}
void Client::sendFrame(Protocol::MessageType type, const QString &requestId,
                               const QByteArray &payload)
{
    if (!isConnected()) {
        emit connectionError("Not connected to server");
        return;
    }

    QByteArray data = Protocol::serializeFrame({type, requestId, payload});
    m_socket->write(data);
    m_socket->flush();
}

void Client::onConnected()
{
    qDebug() << "onConnected";
    emit connected();
}

void Client::onDisconnected()
{
    m_pendingRequests.clear();
    emit disconnected();
    m_timer->start(1'000);
}

void Client::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    processFrame();
}

void Client::onError(QLocalSocket::LocalSocketError socketError)
{
//    Q_UNUSED(socketError)
    if (socketError == QLocalSocket::ServerNotFoundError
            || socketError == QLocalSocket::ConnectionRefusedError) {
        m_timer->start(1'000);
    }
    emit connectionError(m_socket->errorString());
}

void Client::processFrame()
{
    while (m_buffer.size() > sizeof(quint8)) {
        QDataStream stream(m_buffer);
        stream.setVersion(QDataStream::Qt_6_0);

        qint64 startPos = stream.device()->pos();

        try {
            Protocol::Frame frame = Protocol::deserializeFrame(stream);

            switch (frame.type) {
                case Protocol::MessageType::ErrorResponse:
                    handleError(frame.payload);
                    break;
                case Protocol::MessageType::StatusResponse:
                    handleStatusResponse(frame.payload);
                    break;
                case Protocol::MessageType::Heartbeat:
                    break;
                default:
                    break;
            }

            qint64 bytesRead = stream.device()->pos() - startPos;
            m_buffer.remove(0, bytesRead);
        } catch (...) {
            break;
        }
    }
}
void Client::handleError(const QByteArray &payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    QJsonObject error = doc.object();

    QString requestId = error["request_id"].toString();
    QString errorMsg = error["error"].toString();

    emit requestError(requestId, errorMsg);
    m_pendingRequests.remove(requestId);
}

void Client::handleStatusResponse(const QByteArray &payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    emit serverStatusReceived(doc.object());
}
