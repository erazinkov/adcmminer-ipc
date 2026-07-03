#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDataStream>
#include <QString>
#include <QDateTime>

enum class MessageType : quint8 {
    Heartbeat = 1,
    TaskPayload = 2,
    ResultPayload = 3,
    ComplexDataPayload = 4
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

#endif // PROTOCOL_H
