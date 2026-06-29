#include "server.h"
#include <QDataStream>
#include <QDateTime>
#include <QTextStream>
#include <QJsonDocument>
#include <QThread>
#include <iostream>

Server::Server(QObject *parent)
    : QObject{parent}
    , m_server(new QLocalServer(this))
    , m_nextClientId(1)
    , m_threadPool(new QThreadPool(this))
{

    m_controller = new Controller("/misc/agpk_std/adcm.dat");

    // Configure thread pool
    m_threadPool->setMaxThreadCount(QThread::idealThreadCount());

    connect(m_server, &QLocalServer::newConnection, this, &Server::onNewConnection);

    QTextStream(stdout) << "Thread pool configured with "
                       << m_threadPool->maxThreadCount() << " threads\n";
}

Server::~Server()
{
    stop();
}

bool Server::start(const QString &serverName)
{
    QLocalServer::removeServer(serverName);

    if (!m_server->listen(serverName)) {
        emit errorOccurred(m_server->errorString());
        return false;
    }

    emit serverStarted();
    QTextStream(stdout) << "═══════════════════════════════════════════\n";
    QTextStream(stdout) << "  Calculation Server Started\n";
    QTextStream(stdout) << "  Server name: " << serverName << "\n";
    QTextStream(stdout) << "  Max threads: " << m_threadPool->maxThreadCount() << "\n";
    QTextStream(stdout) << "═══════════════════════════════════════════\n";
    QTextStream(stdout) << "Waiting for connections...\n\n";

    return true;
}

void Server::stop()
{
    // Disconnect all clients
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        ClientConnection *client = it.value();
        sendFrame(client->socket, Protocol::MessageType::Shutdown, "",
                 QByteArray("Server is shutting down"));
        client->socket->flush();
        client->socket->disconnectFromServer();
    }

    if (m_server->isListening()) {
        m_server->close();
        emit serverStopped();
    }

    // Clean up
    qDeleteAll(m_clients);
    m_clients.clear();


    QTextStream(stdout) << "\n═══════════════════════════════════════════\n";
    QTextStream(stdout) << "  Server Stopped\n";
    QTextStream(stdout) << "═══════════════════════════════════════════\n";
}

bool Server::isRunning() const
{
    return m_server->isListening();
}

QString Server::errorString() const
{
    return m_server->errorString();
}

int Server::activeConnections() const
{
    return m_clients.size();
}

QString Server::serverName() const
{
    return m_server->serverName();
}

void Server::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();

        ClientConnection *client = new ClientConnection;
        client->socket = socket;
        client->id = m_nextClientId++;
        client->identifier = QString("Client_%1").arg(client->id);

        m_clients[client->id] = client;

        connect(socket, &QLocalSocket::readyRead, this, &Server::onReadyRead);
        connect(socket, &QLocalSocket::disconnected, this, &Server::onDisconnected);

        emit clientConnected(client->id);

        QTextStream(stdout) << "[" << QDateTime::currentDateTime().toString("hh:mm:ss")
                           << "] Client connected: " << client->identifier
                           << " (Total: " << m_clients.size() << ")\n";
    }
}

void Server::onReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
        if (!socket) return;

        // Find client
        ClientConnection *client = nullptr;
        for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
            if (it.value()->socket == socket) {
                client = it.value();
                break;
            }
        }

        if (!client) return;

        // Append new data to buffer
        client->buffer.append(socket->readAll());

        // Process all complete frames
        processFrames(client);

}

void Server::onDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;

    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value()->socket == socket) {
            ClientConnection *client = it.value();

            QTextStream(stdout) << "[" << QDateTime::currentDateTime().toString("hh:mm:ss")
                               << "] Client disconnected: " << client->identifier
                               << " (Total: " << m_clients.size() - 1 << ")\n";

            emit clientDisconnected(client->id);

            m_clients.erase(it);
            delete client;
            break;
        }
    }

    socket->deleteLater();
}

void Server::processFrames(ClientConnection *client)
{
    bool processedFrame = true;

        // Keep processing while we successfully read frames
        while (processedFrame && client->buffer.size() >= sizeof(quint32)) {
            processedFrame = false;

            try {
                int bytesRead = 0;
                Protocol::Frame frame = Protocol::deserializeFrame(client->buffer, bytesRead);

                if (bytesRead > 0) {
                    // Process the frame
                    switch (frame.type) {
                        case Protocol::MessageType::GetStatus:
                            handleGetStatus(client, frame.payload);
                            break;
                        case Protocol::MessageType::Heartbeat:
                            handleHeartbeat(client);
                            break;
                        case Protocol::MessageType::Shutdown:
                            handleShutdown(client);
                            break;
                        default:
                            QTextStream(stderr) << "Unknown message type: "
                                               << (int)frame.type << "\n";
                            break;
                    }

                    // Remove processed data from buffer
                    client->buffer.remove(0, bytesRead);
                    processedFrame = true; // Try to process another frame
                }

            } catch (const std::runtime_error &e) {
                QString errorMsg = e.what();

                if (errorMsg == "Incomplete frame data") {
                    // This is normal - wait for more data
                    // Just break and wait for next readyRead
                    break;
                } else {
                    // Actual error - log it and remove problematic data
                    QTextStream(stderr) << "Frame parsing error: " << errorMsg << "\n";

                    // Try to recover by skipping one byte
                    if (!client->buffer.isEmpty()) {
                        client->buffer.remove(0, 1);
                        processedFrame = true; // Try again after skipping bad byte
                    }
                }
            }
        }
}




void Server::handleGetStatus(ClientConnection *client,
                                       const QByteArray &payload)
{
    Q_UNUSED(payload)

    QJsonObject status;
    status["active_connections"] = m_clients.size();
//    status["active_calculations"] = m_activeCalculators.size();
    status["thread_pool_size"] = m_threadPool->maxThreadCount();
    status["server_uptime"] = "Running...";

    QByteArray responseData = QJsonDocument(status).toJson(QJsonDocument::Compact);

    sendFrame(client->socket, Protocol::MessageType::StatusResponse, "", responseData);
}

void Server::handleHeartbeat(ClientConnection *client)
{
    // Just acknowledge with empty response
    sendFrame(client->socket, Protocol::MessageType::Heartbeat, "", QByteArray());
}

void Server::handleShutdown(ClientConnection *client)
{
    QTextStream(stdout) << "Client " << client->identifier << " requested disconnect\n";
    client->socket->disconnectFromServer();
}

void Server::sendFrame(QLocalSocket *socket, Protocol::MessageType type,
                                 const QString &requestId, const QByteArray &payload)
{
    if (!socket || socket->state() != QLocalSocket::ConnectedState) {
            return;
        }

        Protocol::Frame frame;
        frame.type = type;
        frame.requestId = requestId;
        frame.payload = payload;

        QByteArray data = Protocol::serializeFrame(frame);

        qint64 bytesWritten = socket->write(data);
        if (bytesWritten == -1) {
            QTextStream(stderr) << "Error writing to socket: " << socket->errorString() << "\n";
            return;
        }

        bool flushed = socket->flush();
        if (!flushed) {
            QTextStream(stderr) << "Error flushing socket: " << socket->errorString() << "\n";
        }
}

void Server::sendError(QLocalSocket *socket, const QString &requestId,
                                 const QString &error)
{
    QJsonObject errorObj = Protocol::createResponse(requestId, false, QJsonValue(), error);
    QByteArray data = QJsonDocument(errorObj).toJson(QJsonDocument::Compact);
    sendFrame(socket, Protocol::MessageType::ErrorResponse, requestId, data);
}

void Server::sendProgress(QLocalSocket *socket, const QString &requestId,
                                    int percentage, const QString &message)
{
    QJsonObject progress = Protocol::createProgress(requestId, percentage, message);
    QByteArray data = QJsonDocument(progress).toJson(QJsonDocument::Compact);
    sendFrame(socket, Protocol::MessageType::CalculateProgress, requestId, data);
}

void Server::sendResponse(QLocalSocket *socket, const QString &requestId,
                                    const QJsonValue &result)
{
    QJsonObject response = Protocol::createResponse(requestId, true, result);
    QByteArray data = QJsonDocument(response).toJson(QJsonDocument::Compact);
    sendFrame(socket, Protocol::MessageType::CalculateResponse, requestId, data);
}
