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
        connectToServer(1'000);
    });
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

        Protocol::Frame frame;
        frame.type = type;
        frame.requestId = requestId;
        frame.payload = payload;

        QByteArray data = Protocol::serializeFrame(frame);

        qint64 bytesWritten = m_socket->write(data);
        if (bytesWritten == -1) {
            emit connectionError(QString("Write error: %1").arg(m_socket->errorString()));
            return;
        }

        if (bytesWritten != data.size()) {
            emit connectionError(QString("Incomplete write: %1 of %2 bytes")
                               .arg(bytesWritten).arg(data.size()));
            return;
        }

        m_socket->flush();
}

void Client::onConnected()
{
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
    processFrames();
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

void Client::processFrames()
{
    bool processedFrame = true;

        while (processedFrame && m_buffer.size() >= sizeof(quint32)) {
            processedFrame = false;

            try {
                int bytesRead = 0;
                Protocol::Frame frame = Protocol::deserializeFrame(m_buffer, bytesRead);

                if (bytesRead > 0) {
                    // Process based on type
                    switch (frame.type) {
                        case Protocol::MessageType::ErrorResponse:
                            handleError(frame.payload);
                            break;
                        case Protocol::MessageType::StatusResponse:
                            handleStatusResponse(frame.payload);
                            break;
                        case Protocol::MessageType::Heartbeat:
                            // Heartbeat acknowledged
                            break;
                        default:
                            qDebug() << "Unknown message type:" << (int)frame.type;
                            break;
                    }

                    // Remove processed data
                    m_buffer.remove(0, bytesRead);
                    processedFrame = true;
                }

            } catch (const std::runtime_error &e) {
                QString errorMsg = e.what();

                if (errorMsg == "Incomplete frame data") {
                    // Wait for more data - normal case
                    break;
                } else {
                    qDebug() << "Frame parsing error:" << errorMsg;

                    // Skip problematic byte
                    if (!m_buffer.isEmpty()) {
                        m_buffer.remove(0, 1);
                        processedFrame = true;
                    }
                }
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
    qDebug() << doc;
    emit serverStatusReceived(doc.object());
}
