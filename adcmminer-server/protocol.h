#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDataStream>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QIODevice>

namespace Protocol {
    // Message types
    enum class MessageType : quint8 {
        CalculateRequest = 0x01,
        CalculateResponse = 0x02,
        CalculateProgress = 0x03,
        CancelCalculation = 0x04,
        GetStatus = 0x05,
        StatusResponse = 0x06,
        ErrorResponse = 0x07,
        Heartbeat = 0x08,
        Shutdown = 0xFF
    };

    // Calculation types
    namespace Calculations {
        const QString PRIME_FACTORIZATION = "prime_factorization";
        const QString FIBONACCI = "fibonacci";
        const QString PI_DIGITS = "pi_digits";
        const QString MATRIX_MULTIPLY = "matrix_multiply";
        const QString SORT_LARGE_ARRAY = "sort_large_array";
        const QString HASH_COMPUTATION = "hash_computation";
        const QString MONTE_CARLO_SIMULATION = "monte_carlo";
    }

    // Protocol frame structure
    struct Frame {
        MessageType type;
        QString requestId;
        QByteArray payload;
    };

    // Serialization helpers
    inline QByteArray serializeFrame(const Frame &frame) {
        QByteArray block;
        QDataStream stream(&block, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_6_0);

        stream << (quint8)frame.type;
        stream << frame.requestId;
        stream << frame.payload;

        return block;
    }

    inline Frame deserializeFrame(QDataStream &stream) {
        Frame frame;
        quint8 type;
        stream >> type;
        frame.type = static_cast<MessageType>(type);
        stream >> frame.requestId;
        stream >> frame.payload;
        return frame;
    }

    // Create request
    inline QJsonObject createRequest(const QString &calculationType,
                                     const QJsonObject &params) {
        QJsonObject request;
        request["calculation_type"] = calculationType;
        request["params"] = params;
        return request;
    }

    // Create response
    inline QJsonObject createResponse(const QString &requestId,
                                     bool success,
                                     const QJsonValue &result,
                                     const QString &error = "") {
        QJsonObject response;
        response["request_id"] = requestId;
        response["success"] = success;
        response["result"] = result;
        if (!error.isEmpty()) {
            response["error"] = error;
        }
        response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    }

    // Create progress update
    inline QJsonObject createProgress(const QString &requestId,
                                     int percentage,
                                     const QString &message = "") {
        QJsonObject progress;
        progress["request_id"] = requestId;
        progress["percentage"] = percentage;
        progress["message"] = message;
        return progress;
    }
}

#endif // PROTOCOL_H
