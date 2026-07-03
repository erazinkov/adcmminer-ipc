#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDataStream>
#include <QString>
#include <QDateTime>

enum class MessageType : quint8 {
    Heartbeat = 1,
    TaskPayload = 2,
    ResultPayload = 3,
    ComplexDataPayload = 4,
    ComplexDataPayloadTime = 5
};

struct TaskData {
    int id;
    QString title;
    QDateTime deadline;
    bool isCompleted;
};

// Перегрузка оператора вывода (сериализация в поток)
inline QDataStream &operator<<(QDataStream &out, const TaskData &task) {
    out << task.id << task.title << task.deadline << task.isCompleted;
    return out;
}

// Перегрузка оператора ввода (десериализация из потока)
inline QDataStream &operator>>(QDataStream &in, TaskData &task) {
    in >> task.id >> task.title >> task.deadline >> task.isCompleted;
    return in;
}

struct ResultData {
    int id;
    QString title;
    QDateTime deadline;
    bool isCompleted;
};

// Перегрузка оператора вывода (сериализация в поток)
inline QDataStream &operator<<(QDataStream &out, const ResultData &result) {
    out << result.id << result.title << result.deadline << result.isCompleted;
    return out;
}

// Перегрузка оператора ввода (десериализация из потока)
inline QDataStream &operator>>(QDataStream &in, ResultData &result) {
    in >> result.id >> result.title >> result.deadline >> result.isCompleted;
    return in;
}

//#include <QDataStream>
//#include <QString>
//#include <QJsonObject>
//#include <QJsonArray>
//#include <QUuid>
//#include <QIODevice>

//namespace Protocol {
//    // Message types
//    enum class MessageType : quint8 {
//        CalculateRequest = 0x01,
//        CalculateResponse = 0x02,
//        CalculateProgress = 0x03,
//        CancelCalculation = 0x04,
//        GetStatus = 0x05,
//        StatusResponse = 0x06,
//        ErrorResponse = 0x07,
//        Heartbeat = 0x08,
//        Shutdown = 0xFF
//    };

//    // Calculation types
//    namespace Calculations {
//        const QString PRIME_FACTORIZATION = "prime_factorization";
//        const QString FIBONACCI = "fibonacci";
//        const QString PI_DIGITS = "pi_digits";
//        const QString MATRIX_MULTIPLY = "matrix_multiply";
//        const QString SORT_LARGE_ARRAY = "sort_large_array";
//        const QString HASH_COMPUTATION = "hash_computation";
//        const QString MONTE_CARLO_SIMULATION = "monte_carlo";
//    }

//    // Protocol frame structure
//    struct Frame {
//        MessageType type;
//        QString requestId;
//        QByteArray payload;
//    };

//    // Serialization helpers
//    inline QByteArray serializeFrame(const Frame &frame) {
//        QByteArray block;
//        QDataStream stream(&block, QIODevice::WriteOnly);
//        stream.setVersion(QDataStream::Qt_6_0);

//        // Create payload data
//        QByteArray payloadData;
//        QDataStream payloadStream(&payloadData, QIODevice::WriteOnly);
//        payloadStream.setVersion(QDataStream::Qt_6_0);
//        payloadStream << (quint8)frame.type;
//        payloadStream << frame.requestId;
//        payloadStream << frame.payload;

//        // Write total size first (4 bytes for size + payload)
//        quint32 totalSize = sizeof(quint32) + payloadData.size();
//        stream << totalSize;

//        // Write the actual payload
//        stream.writeRawData(payloadData.constData(), payloadData.size());

//        return block;
//    }

//    inline Frame deserializeFrame(const QByteArray &data, int &bytesRead) {
//        Frame frame;
//                bytesRead = 0;

//                if (data.size() < sizeof(quint32)) {
//                    throw std::runtime_error("Not enough data for size field");
//                }

//                // Read total size (including the size field itself)
//                QDataStream sizeStream(data.left(sizeof(quint32)));
//                sizeStream.setVersion(QDataStream::Qt_6_0);
//                quint32 totalSize;
//                sizeStream >> totalSize;

//                // Check if we have enough data
//                if (data.size() < totalSize) {
//                    throw std::runtime_error("Incomplete frame data");
//                }

//                // Extract payload (everything after size field)
//                QByteArray payloadData = data.mid(sizeof(quint32), totalSize - sizeof(quint32));

//                // Parse payload
//                QDataStream payloadStream(payloadData);
//                payloadStream.setVersion(QDataStream::Qt_6_0);

//                quint8 type;
//                payloadStream >> type;
//                frame.type = static_cast<MessageType>(type);
//                payloadStream >> frame.requestId;
//                payloadStream >> frame.payload;

//                // Report how many bytes were consumed
//                bytesRead = totalSize;

//                return frame;
//    }

//    // Create request
//    inline QJsonObject createRequest(const QString &calculationType,
//                                     const QJsonObject &params) {
//        QJsonObject request;
//        request["calculation_type"] = calculationType;
//        request["params"] = params;
//        return request;
//    }

//    // Create response
//    inline QJsonObject createResponse(const QString &requestId,
//                                     bool success,
//                                     const QJsonValue &result,
//                                     const QString &error = "") {
//        QJsonObject response;
//        response["request_id"] = requestId;
//        response["success"] = success;
//        response["result"] = result;
//        if (!error.isEmpty()) {
//            response["error"] = error;
//        }
//        response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
//        return response;
//    }

//    // Create progress update
//    inline QJsonObject createProgress(const QString &requestId,
//                                     int percentage,
//                                     const QString &message = "") {
//        QJsonObject progress;
//        progress["request_id"] = requestId;
//        progress["percentage"] = percentage;
//        progress["message"] = message;
//        return progress;
//    }
//}

#endif // PROTOCOL_H
