#include "server.h"
#include <QDataStream>
#include <QDateTime>
#include <iostream>
#include <QPointF>


Server::Server(QObject *parent)
    : QObject{parent}

{
    m_server = new QLocalServer(this);
    m_controller = new Controller("/misc/agpk_std/adcm.dat");

    connect(m_server, &QLocalServer::newConnection, this, &Server::onNewConnection);
}


bool Server::start(const QString &serverName) {
    QLocalServer::removeServer(serverName);
    if (!m_server->listen(serverName)) {
        emit serverError(m_server->errorString());
        return false;
    }
    emit serverStarted();
    return true;
}

void Server::onNewConnection() {
//    while (m_server->hasPendingConnections()) {
//        QLocalSocket *clientSocket = m_server->nextPendingConnection();
//        if (!clientSocket) continue;

//        connect(clientSocket, &QLocalSocket::readyRead, this, &Server::onReadyRead);
//        connect(clientSocket, &QLocalSocket::disconnected, this, &Server::onClientDisconnected);

//        qDebug() << "Server accepted a new client connection.";
//    }
    while (m_server->hasPendingConnections()) {
            QLocalSocket *clientSocket = m_server->nextPendingConnection();
            if (!clientSocket) continue;

            // Создаем таймер таймаута специально для этого клиента
            QTimer *clientTimer = new QTimer(clientSocket);
            clientTimer->setInterval(10'000);
            clientTimer->setSingleShot(true);

            // Если таймер сработал — клиент признается мертвым
            connect(clientTimer, &QTimer::timeout, this, [clientSocket]() {
                qWarning() << "Клиент молчит слишком долго. Принудительное отключение.";
                clientSocket->disconnectFromServer();
            });

            connect(clientSocket, &QLocalSocket::readyRead, this, [this, clientSocket, clientTimer]() {
                // Сбрасываем и перезапускаем таймер, так как клиент подал признаки жизни
                qDebug() << "Сбрасываем и перезапускаем таймер, так как клиент подал признаки жизни";
                clientTimer->start();
                this->processIncomingData(clientSocket);
            });

            connect(clientSocket, &QLocalSocket::disconnected, this, &Server::onClientDisconnected);

            // Запускаем отсчет сразу после подключения
            clientTimer->start();
            qDebug() << "Новое подключение. Таймер контроля запущен.";
        }

}

void Server::processIncomingData(QLocalSocket *clientSocket) {
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    while (true) {
        in.startTransaction();

        quint32 blockSize = 0;
        quint8 msgTypeRaw = 0;

        in >> blockSize;
        in >> msgTypeRaw;

        if (!in.commitTransaction()) {
            break; // Ждем догрузки пакета
        }

        MessageType type = static_cast<MessageType>(msgTypeRaw);

        if (type == MessageType::Heartbeat) {
            // Пакет успешно прочитан. Делать ничего не нужно,
            // так как таймер clientTimer уже перезапустился в лямбде выше.
            qDebug() << "Получен Heartbeat от клиента.";
            if (clientSocket && clientSocket->state() == QLocalSocket::ConnectedState) {
                this->sendHeartbeatToClient(clientSocket);
            }
        }
        else if (type == MessageType::TaskPayload) {
            // Читаем полезную нагрузку (например, структуру TaskData из прошлого шага)
            TaskData task;
            in.startTransaction(); // Открываем подтранзакцию для тела сообщения
            in >> task;

            if (in.commitTransaction()) {
                qDebug() << "Получена задача:" << task.title;
            }


            ResultData result;
            result.id = task.id;
            result.title = task.title;
            result.deadline = task.deadline;
            result.isCompleted = true;
            if (clientSocket && clientSocket->state() == QLocalSocket::ConnectedState) {
                this->sendResultToClient(clientSocket, result);
            }

            qDebug() << "Сервер успешно принял структуру TaskData:";
            qDebug() << "ID:" << task.id;
            qDebug() << "Title:" << task.title;
            qDebug() << "Deadline:" << task.deadline.toString();
            qDebug() << "Status (Completed):" << task.isCompleted;
        }
    }
}


void Server::sendHeartbeatToClient(QLocalSocket *clientSocket) {
    // 1. Проверяем, что клиент действительно подключен
    if (!clientSocket || clientSocket->state() != QLocalSocket::ConnectedState) {
        return;
    }

    // 2. Создаем байтовый массив (буфер) для сборки пакета
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    // 3. Резервируем первые 4 байта под размер пакета (записываем туда 0)
    out << quint32(0);

    // 4. Записываем тип сообщения (Heartbeat)
    out << static_cast<quint8>(MessageType::Heartbeat);

    // Подсказка: Если вам нужно передать дополнительную информацию
    // (например, timestamp сервера), её можно записать прямо здесь:
    // out << QDateTime::currentMSecsSinceEpoch();

    // 5. Возвращаемся в начало структуры данных и перезаписываем точный размер пакета.
    // Размер равен: общая длина блока минус 4 байта, занятые самим заголовком размера.
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));

    // 6. Записываем пакет в системный буфер сокета (асинхронная неблокирующая операция)
    clientSocket->write(block);

    qDebug() << "Сервер отправил Heartbeat пакет размером:" << block.size() << "байт.";
}

void Server::sendResultToClient(QLocalSocket *clientSocket, const ResultData &result) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    out << quint32(0);
    out << static_cast<quint8>(MessageType::ResultPayload);
    out << result; // Записываем структуру с результатом

    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));

    clientSocket->write(block);
    qDebug() << "Результат вычислений для задачи #" << result.id << "отправлен клиенту.";
}

void Server::sendComplexDataToClient(QLocalSocket *clientSocket,
                                          const QMap<QString, QList<QPointF>> &data,
                                          const QMap<QString, QStringList> &text)
{
    if (!clientSocket || clientSocket->state() != QLocalSocket::ConnectedState) {
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion); // Фиксируем версию Qt для сериализации [1]

    // 1. Резервируем 4 байта под размер пакета [1]
    out << quint32(0);

    // 2. Записываем тип сообщения [1]
    out << static_cast<quint8>(MessageType::ComplexDataPayload);

    // 3. Записываем сами коллекции (Qt автоматически сериализует QMap, QList и QPointF) [1]
    out << data;
    out << text;

    // 4. Возвращаемся в начало и перезаписываем точный размер тела пакета [1]
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));

    // 5. Неблокирующая отправка в буфер [1]
    clientSocket->write(block);
    clientSocket->flush();
}

void Server::onClientDisconnected() {
    auto *clientSocket = qobject_cast<QLocalSocket*>(sender());
    if (!clientSocket) return;

    qDebug() << "Client connection closed.";
    clientSocket->deleteLater();
}

bool Server::isRunning() const
{
    return m_server->isListening();
}

//bool Server::start(const QString &serverName)
//{
//    QLocalServer::removeServer(serverName);

//    if (!m_server->listen(serverName)) {
//        emit errorOccurred(m_server->errorString());
//        return false;
//    }

//    emit serverStarted();
//    QTextStream(stdout) << "═══════════════════════════════════════════\n";
//    QTextStream(stdout) << "  Calculation Server Started\n";
//    QTextStream(stdout) << "  Server name: " << serverName << "\n";
//    QTextStream(stdout) << "  Max threads: " << m_threadPool->maxThreadCount() << "\n";
//    QTextStream(stdout) << "═══════════════════════════════════════════\n";
//    QTextStream(stdout) << "Waiting for connections...\n\n";

//    return true;
//}

//void Server::stop()
//{
//    // Disconnect all clients
//    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
//        ClientConnection *client = it.value();
//        sendFrame(client->socket, Protocol::MessageType::Shutdown, "",
//                 QByteArray("Server is shutting down"));
//        client->socket->flush();
//        client->socket->disconnectFromServer();
//    }

//    if (m_server->isListening()) {
//        m_server->close();
//        emit serverStopped();
//    }

//    // Clean up
//    qDeleteAll(m_clients);
//    m_clients.clear();


//    QTextStream(stdout) << "\n═══════════════════════════════════════════\n";
//    QTextStream(stdout) << "  Server Stopped\n";
//    QTextStream(stdout) << "═══════════════════════════════════════════\n";
//}



//QString Server::errorString() const
//{
//    return m_server->errorString();
//}

//int Server::activeConnections() const
//{
//    return m_clients.size();
//}

//QString Server::serverName() const
//{
//    return m_server->serverName();
//}

//void Server::onNewConnection()
//{
//    while (m_server->hasPendingConnections()) {
//        QLocalSocket *socket = m_server->nextPendingConnection();

//        ClientConnection *client = new ClientConnection;
//        client->socket = socket;
//        client->id = m_nextClientId++;
//        client->identifier = QString("Client_%1").arg(client->id);

//        m_clients[client->id] = client;

//        connect(socket, &QLocalSocket::readyRead, this, &Server::onReadyRead);
//        connect(socket, &QLocalSocket::disconnected, this, &Server::onDisconnected);

//        emit clientConnected(client->id);

//        QTextStream(stdout) << "[" << QDateTime::currentDateTime().toString("hh:mm:ss")
//                           << "] Client connected: " << client->identifier
//                           << " (Total: " << m_clients.size() << ")\n";
//    }
//}

//void Server::onReadyRead()
//{
//    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
//        if (!socket) return;

//        // Find client
//        ClientConnection *client = nullptr;
//        for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
//            if (it.value()->socket == socket) {
//                client = it.value();
//                break;
//            }
//        }

//        if (!client) return;

//        // Append new data to buffer
//        client->buffer.append(socket->readAll());

//        // Process all complete frames
//        processFrames(client);

//}

//void Server::onDisconnected()
//{
//    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
//    if (!socket) return;

//    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
//        if (it.value()->socket == socket) {
//            ClientConnection *client = it.value();

//            QTextStream(stdout) << "[" << QDateTime::currentDateTime().toString("hh:mm:ss")
//                               << "] Client disconnected: " << client->identifier
//                               << " (Total: " << m_clients.size() - 1 << ")\n";

//            emit clientDisconnected(client->id);

//            m_clients.erase(it);
//            delete client;
//            break;
//        }
//    }

//    socket->deleteLater();
//}

//void Server::processFrames(ClientConnection *client)
//{
//    bool processedFrame = true;

//        // Keep processing while we successfully read frames
//        while (processedFrame && client->buffer.size() >= sizeof(quint32)) {
//            processedFrame = false;

//            try {
//                int bytesRead = 0;
//                Protocol::Frame frame = Protocol::deserializeFrame(client->buffer, bytesRead);

//                if (bytesRead > 0) {
//                    // Process the frame
//                    switch (frame.type) {
//                        case Protocol::MessageType::GetStatus:
//                            handleGetStatus(client, frame.payload);
//                            break;
//                        case Protocol::MessageType::Heartbeat:
//                            handleHeartbeat(client);
//                            break;
//                        case Protocol::MessageType::Shutdown:
//                            handleShutdown(client);
//                            break;
//                        default:
//                            QTextStream(stderr) << "Unknown message type: "
//                                               << (int)frame.type << "\n";
//                            break;
//                    }

//                    // Remove processed data from buffer
//                    client->buffer.remove(0, bytesRead);
//                    processedFrame = true; // Try to process another frame
//                }

//            } catch (const std::runtime_error &e) {
//                QString errorMsg = e.what();

//                if (errorMsg == "Incomplete frame data") {
//                    // This is normal - wait for more data
//                    // Just break and wait for next readyRead
//                    break;
//                } else {
//                    // Actual error - log it and remove problematic data
//                    QTextStream(stderr) << "Frame parsing error: " << errorMsg << "\n";

//                    // Try to recover by skipping one byte
//                    if (!client->buffer.isEmpty()) {
//                        client->buffer.remove(0, 1);
//                        processedFrame = true; // Try again after skipping bad byte
//                    }
//                }
//            }
//        }
//}




//void Server::handleGetStatus(ClientConnection *client,
//                                       const QByteArray &payload)
//{
//    Q_UNUSED(payload)

//    QJsonObject status;
//    status["active_connections"] = m_clients.size();
////    status["active_calculations"] = m_activeCalculators.size();
//    status["thread_pool_size"] = m_threadPool->maxThreadCount();
//    status["server_uptime"] = "Running...";

//    QByteArray responseData = QJsonDocument(status).toJson(QJsonDocument::Compact);

//    sendFrame(client->socket, Protocol::MessageType::StatusResponse, "", responseData);
//}

//void Server::handleHeartbeat(ClientConnection *client)
//{
//    // Just acknowledge with empty response
//    sendFrame(client->socket, Protocol::MessageType::Heartbeat, "", QByteArray());
//}

//void Server::handleShutdown(ClientConnection *client)
//{
//    QTextStream(stdout) << "Client " << client->identifier << " requested disconnect\n";
//    client->socket->disconnectFromServer();
//}

//void Server::sendFrame(QLocalSocket *socket, Protocol::MessageType type,
//                                 const QString &requestId, const QByteArray &payload)
//{
//    if (!socket || socket->state() != QLocalSocket::ConnectedState) {
//            return;
//        }

//        Protocol::Frame frame;
//        frame.type = type;
//        frame.requestId = requestId;
//        frame.payload = payload;

//        QByteArray data = Protocol::serializeFrame(frame);

//        qint64 bytesWritten = socket->write(data);
//        if (bytesWritten == -1) {
//            QTextStream(stderr) << "Error writing to socket: " << socket->errorString() << "\n";
//            return;
//        }

//        bool flushed = socket->flush();
//        if (!flushed) {
//            QTextStream(stderr) << "Error flushing socket: " << socket->errorString() << "\n";
//        }
//}

//void Server::sendError(QLocalSocket *socket, const QString &requestId,
//                                 const QString &error)
//{
//    QJsonObject errorObj = Protocol::createResponse(requestId, false, QJsonValue(), error);
//    QByteArray data = QJsonDocument(errorObj).toJson(QJsonDocument::Compact);
//    sendFrame(socket, Protocol::MessageType::ErrorResponse, requestId, data);
//}

//void Server::sendProgress(QLocalSocket *socket, const QString &requestId,
//                                    int percentage, const QString &message)
//{
//    QJsonObject progress = Protocol::createProgress(requestId, percentage, message);
//    QByteArray data = QJsonDocument(progress).toJson(QJsonDocument::Compact);
//    sendFrame(socket, Protocol::MessageType::CalculateProgress, requestId, data);
//}

//void Server::sendResponse(QLocalSocket *socket, const QString &requestId,
//                                    const QJsonValue &result)
//{
//    QJsonObject response = Protocol::createResponse(requestId, true, result);
//    QByteArray data = QJsonDocument(response).toJson(QJsonDocument::Compact);
//    sendFrame(socket, Protocol::MessageType::CalculateResponse, requestId, data);
//}
