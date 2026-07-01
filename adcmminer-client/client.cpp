#include "client.h"

#include <QDataStream>
#include <QPointF>
#include <iostream>

#include "protocol.h"

Client::Client(QObject *parent)
    : QObject(parent)
{
    m_socket = new QLocalSocket(this);

   m_heartbeatTimer = new QTimer(this);
   m_heartbeatTimer->setSingleShot(true);
//   connect(m_heartbeatTimer, &QTimer::timeout, this, &Client::sendHeartbeat);
   connect(m_heartbeatTimer, &QTimer::timeout, this, &Client::handleHeartbeatTimeout);

   connect(m_socket, &QLocalSocket::connected, this, &Client::onConnected);
   connect(m_socket, &QLocalSocket::disconnected, this, &Client::onDisconnected);
   connect(m_socket, &QLocalSocket::readyRead, this, &Client::onReadyRead);
   connect(m_socket, &QLocalSocket::errorOccurred, this, &Client::onErrorOccurred);

   m_reconnectTimer = new QTimer(this);
   connect(m_reconnectTimer, &QTimer::timeout, this, &Client::attemptReconnect);

   m_timer = new QTimer(this);
   m_timer->setInterval(HEARTBEAT_INTERVAL_MS);
   connect(m_timer, &QTimer::timeout, this, &Client::sendHeartbeat);

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
    attemptReconnect();
}

void Client::onConnected() {

//    qDebug() << "Client connected asynchronously!";
//    emit connected();
    qDebug() << "Успешно подключено к серверу!";
    m_reconnectTimer->stop(); // Подключились -> перестаем долбиться реконнектами
    m_heartbeatTimer->start(HEARTBEAT_TIMEOUT_MS);
    m_timer->start();
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
    qDebug() << "Соединение разорвано со стороны сервера.";
    m_heartbeatTimer->stop();
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    // Запускаем таймер реконнекта, если он еще не запущен
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start(RECONNECT_INTERVAL_MS);
    }
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



        // Проверяем, удалось ли прочитать хотя бы заголовок
        // Если данных в сокете не хватило даже на заголовок, commitTransaction вернет false
        if (!in.commitTransaction()) {
            // Выходим из цикла и ждем следующего вызова readyRead()
            break;
        }

        m_heartbeatTimer->start(HEARTBEAT_TIMEOUT_MS);

        // Преобразуем число в строго типизированный enum
        MessageType type = static_cast<MessageType>(msgTypeRaw);



        // Вторая транзакция: для чтения тела конкретного сообщения
        in.startTransaction();

        if (type == MessageType::ComplexDataPayload) {
            in.startTransaction(); // Начинаем транзакцию для чтения тела данных [1]

            // Создаем пустые объекты для приема [1]
            QMap<QString, QList<QPointF>> receivedData;
            QMap<QString, QStringList> receivedText;

            // Читаем данные строго в том же порядке, в каком записывали! [1]
            in >> receivedData;
            in >> receivedText;

            if (!in.commitTransaction()) {
                // Если сеть не успела передать все элементы QMap,
                // транзакция откатится, и мы вернемся сюда при следующем readyRead [1]
                break;
            }

            // Данные успешно и полностью получены без блокировки потока [1]
            qDebug() << "Получены комплексные данные.";
            qDebug() << "Ключей в карте координат:" << receivedData.size();
            qDebug() << "Ключей в карте текстов:" << receivedText.size();

            // Передаем данные дальше (например, генерируем сигнал для UI) [1]
            emit complexDataReceived(receivedData, receivedText);
        }
        else if (type == MessageType::ResultPayload) {
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
    qDebug() << "Ошибка сокета:" << m_socket->errorString();

    // При ошибке «Сервер не найден / Отклонено» Qt не всегда вызывает disconnected(),
    // поэтому подстраховываемся и запускаем реконнект здесь
    m_heartbeatTimer->stop();
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start(RECONNECT_INTERVAL_MS);
    }
//    emit connectionError(m_socket->errorString());
//    qDebug() << "Client Socket Error:" << m_socket->errorString();
}

void Client::attemptReconnect()
{
    if (m_socket->state() == QLocalSocket::ConnectedState ||
        m_socket->state() == QLocalSocket::ConnectingState) {
        return;
    }

    qDebug() << "Попытка подключения к серверу" << m_serverName << "...";
    m_socket->connectToServer(m_serverName);
}

void Client::handleHeartbeatTimeout()
{
    m_socket->abort();
}

void Client::disconnectFromServer()
{
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
