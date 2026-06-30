#include "client.h"

#include <QDataStream>
#include <iostream>

#include "protocol.h"

Client::Client(QObject *parent)
    : QObject(parent)
{
    m_socket = new QLocalSocket(this);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(5'000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &Client::sendHeartbeat);

   connect(m_socket, &QLocalSocket::connected, this, &Client::onConnected);
   connect(m_socket, &QLocalSocket::disconnected, this, &Client::onDisconnected);
   connect(m_socket, &QLocalSocket::readyRead, this, &Client::onReadyRead);
   connect(m_socket, &QLocalSocket::errorOccurred, this, &Client::onErrorOccurred);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);

//    connect(m_timer, &QTimer::timeout, this, [&](){
//        if (m_socket->state() == QLocalSocket::ConnectedState) {
//            return;
//        }
//        qDebug() << "Try to reconnect" << m_serverName;
//        m_socket->connectToServer(m_serverName);
//    });
}

Client::~Client()
{
    disconnectFromServer();
}

void Client::connectToServer(const QString &serverName)
{
    m_serverName = serverName;
    if (m_socket->state() == QLocalSocket::ConnectedState) {
        return;
    }
    m_socket->connectToServer(m_serverName);
}

void Client::onConnected() {
    m_heartbeatTimer->start();
    m_timer->stop();
    qDebug() << "Client connected asynchronously!";
    emit connected();
}

void Client::sendHeartbeat() {
    if (m_socket->state() != QLocalSocket::ConnectedState) return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    // Заголовок размера (4 байта) + Тип сообщения (1 байт)
    out << quint32(0);
    out << static_cast<quint8>(MessageType::Heartbeat);

    // Перезаписываем размер пакета
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));

    m_socket->write(block);
    // Для экономии ресурсов вызывать flush() не обязательно,
    // Qt сам отправит байты при возврате в event loop.
}

void Client::onDisconnected()
{
    m_heartbeatTimer->stop();
    m_timer->start(1'000);
}

void Client::onReadyRead() {
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    // Цикл работает до тех пор, пока в буфере есть полностью сформированные пакеты
    while (true) {
        // Открываем транзакцию Qt для безопасного чтения
        in.startTransaction();

        quint32 blockSize = 0;
        quint8 msgTypeRaw = 0;

        // Читаем базовый заголовок (размер пакета и тип сообщения)
        in >> blockSize;
        in >> msgTypeRaw;

        // Преобразуем число в строго типизированный enum
        MessageType type = static_cast<MessageType>(msgTypeRaw);

        // Проверяем, удалось ли прочитать хотя бы заголовок
        // Если данных в сокете не хватило даже на заголовок, commitTransaction вернет false
        if (!in.commitTransaction()) {
            // Выходим из цикла и ждем следующего вызова readyRead()
            break;
        }

        // Вторая транзакция: для чтения тела конкретного сообщения
        in.startTransaction();

        if (type == MessageType::ResultPayload) {
            ResultData result;

            // Используем перегруженный оператор >> для десериализации структуры
            in >> result;

            // Проверяем, пришли ли ВСЕ байты, соответствующие этой структуре
            if (!in.commitTransaction()) {
                // Пакет пришел не полностью. Откатываемся и ждем догрузки данных.
                break;
            }

            // Данные успешно собраны без блокировки потока!
            qDebug() << "Успех! Получен результат для задачи #" << result.id
                     << "Значение:" << result.deadline.toLocalTime();

            // Генерируем сигнал для отправки данных в UI или бизнес-логику
            emit resultReceived(result);
        }
        else if (type == MessageType::Heartbeat) {
            // Если сервер прислал подтверждение активности (Heartbeat)
            if (!in.commitTransaction()) break;

            qDebug() << "Клиент зафиксировал Heartbeat от сервера.";
            // Здесь можно обнулять таймер таймаута соединения
        }
        else {
            // Обработка неизвестного типа сообщения во избежание зацикливания
            if (!in.commitTransaction()) break;
            qDebug() << "Получен неизвестный тип сообщения:" << msgTypeRaw;
        }
    }
}

void Client::onErrorOccurred(QLocalSocket::LocalSocketError socketError) {
    if (socketError == QLocalSocket::ServerNotFoundError
            || socketError == QLocalSocket::ConnectionRefusedError) {
        m_timer->start(1'000);
    }
    emit connectionError(m_socket->errorString());
    qDebug() << "Client Socket Error:" << m_socket->errorString();
}

void Client::disconnectFromServer()
{
    m_timer->stop();
    if (m_socket->state() == QLocalSocket::ConnectedState) {
        m_socket->disconnectFromServer();
        emit disconnected();
    }
}

void Client::sendTask(const TaskData &task) {
    if (m_socket->state() != QLocalSocket::ConnectedState) {
        qDebug() << "Нет подключения к серверу.";
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    // Заголовок размера (4 байта) + Тип сообщения (1 байт)
    out << quint32(0);
    out << static_cast<quint8>(MessageType::TaskPayload);

    // Записываем нашу кастомную структуру (сработает перегруженный оператор <<)
    out << task;

    // Возвращаемся в начало и перезаписываем точный размер данных
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));

    // Отправляем байты в буфер ОС
    m_socket->write(block);
}

//bool Client::isConnected() const
//{
//    return m_socket->state() == QLocalSocket::ConnectedState;
//}

//void Client::requestServerStatus()
//{
//    sendFrame(Protocol::MessageType::GetStatus, "", QByteArray());
//}

//void Client::sendHeartbeat()
//{
//    sendFrame(Protocol::MessageType::Heartbeat, "", QByteArray());
//}
//void Client::sendFrame(Protocol::MessageType type, const QString &requestId,
//                               const QByteArray &payload)
//{
//    if (!isConnected()) {
//            emit connectionError("Not connected to server");
//            return;
//        }

//        Protocol::Frame frame;
//        frame.type = type;
//        frame.requestId = requestId;
//        frame.payload = payload;

//        QByteArray data = Protocol::serializeFrame(frame);

//        qint64 bytesWritten = m_socket->write(data);
//        if (bytesWritten == -1) {
//            emit connectionError(QString("Write error: %1").arg(m_socket->errorString()));
//            return;
//        }

//        if (bytesWritten != data.size()) {
//            emit connectionError(QString("Incomplete write: %1 of %2 bytes")
//                               .arg(bytesWritten).arg(data.size()));
//            return;
//        }

//        m_socket->flush();
//}

//void Client::onConnected()
//{
//    emit connected();
//}

//void Client::onDisconnected()
//{
//    m_pendingRequests.clear();
//    emit disconnected();
//    m_timer->start(1'000);
//}

//void Client::onReadyRead()
//{
//    m_buffer.append(m_socket->readAll());
//    processFrames();
//}

//void Client::onError(QLocalSocket::LocalSocketError socketError)
//{
////    Q_UNUSED(socketError)
//    if (socketError == QLocalSocket::ServerNotFoundError
//            || socketError == QLocalSocket::ConnectionRefusedError) {
//        m_timer->start(1'000);
//    }
//    emit connectionError(m_socket->errorString());
//}

//void Client::processFrames()
//{
//    bool processedFrame = true;

//        while (processedFrame && m_buffer.size() >= sizeof(quint32)) {
//            processedFrame = false;

//            try {
//                int bytesRead = 0;
//                Protocol::Frame frame = Protocol::deserializeFrame(m_buffer, bytesRead);

//                if (bytesRead > 0) {
//                    // Process based on type
//                    switch (frame.type) {
//                        case Protocol::MessageType::ErrorResponse:
//                            handleError(frame.payload);
//                            break;
//                        case Protocol::MessageType::StatusResponse:
//                            handleStatusResponse(frame.payload);
//                            break;
//                        case Protocol::MessageType::Heartbeat:
//                            // Heartbeat acknowledged
//                            break;
//                        default:
//                            qDebug() << "Unknown message type:" << (int)frame.type;
//                            break;
//                    }

//                    // Remove processed data
//                    m_buffer.remove(0, bytesRead);
//                    processedFrame = true;
//                }

//            } catch (const std::runtime_error &e) {
//                QString errorMsg = e.what();

//                if (errorMsg == "Incomplete frame data") {
//                    // Wait for more data - normal case
//                    break;
//                } else {
//                    qDebug() << "Frame parsing error:" << errorMsg;

//                    // Skip problematic byte
//                    if (!m_buffer.isEmpty()) {
//                        m_buffer.remove(0, 1);
//                        processedFrame = true;
//                    }
//                }
//            }
//        }
//}
//void Client::handleError(const QByteArray &payload)
//{
//    QJsonDocument doc = QJsonDocument::fromJson(payload);
//    QJsonObject error = doc.object();

//    QString requestId = error["request_id"].toString();
//    QString errorMsg = error["error"].toString();

//    emit requestError(requestId, errorMsg);
//    m_pendingRequests.remove(requestId);
//}

//void Client::handleStatusResponse(const QByteArray &payload)
//{
//    QJsonDocument doc = QJsonDocument::fromJson(payload);
//    qDebug() << doc;
//    emit serverStatusReceived(doc.object());
//}
