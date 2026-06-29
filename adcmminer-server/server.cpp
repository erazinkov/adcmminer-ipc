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

    client->buffer.append(socket->readAll());

    // Process complete frames
    while (client->buffer.size() > sizeof(quint8)) {
        QDataStream stream(client->buffer);
        stream.setVersion(QDataStream::Qt_6_0);

        qint64 startPos = stream.device()->pos();

        try {
            processFrame(client);

            qint64 bytesRead = stream.device()->pos() - startPos;
            client->buffer.remove(0, bytesRead);
        } catch (...) {
            // Incomplete frame, wait for more data
            break;
        }
    }
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

void Server::processFrame(ClientConnection *client)
{
    QDataStream stream(client->buffer);
    stream.setVersion(QDataStream::Qt_6_0);

    Protocol::Frame frame = Protocol::deserializeFrame(stream);

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
            QTextStream(stderr) << "Unknown message type\n";
            break;
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
    Protocol::Frame frame;
    frame.type = type;
    frame.requestId = requestId;
    frame.payload = payload;

    QByteArray data = Protocol::serializeFrame(frame);
    socket->write(data);
    socket->flush();
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
