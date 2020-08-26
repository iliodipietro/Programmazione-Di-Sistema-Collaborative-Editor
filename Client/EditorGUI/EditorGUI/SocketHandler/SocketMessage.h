#pragma once
#include <QObject>
#include <QIODevice>
#include <QDataStream>
#include <QSharedPointer>

#include "Structures/MessageTypes.h"

class SocketMessage : public QObject{
    Q_OBJECT

private:
    //qint64 m_size;
    MessageTypes m_messageType;
    QByteArray m_message;

    QByteArray intToArray(qint32 source);
    qint32 arrayToInt(QByteArray source);

public:

    SocketMessage(MessageTypes messageType, QByteArray message):
        m_messageType(messageType), m_message(message){}

    SocketMessage(QByteArray message);

    //static QSharedPointer<SocketMessage> deserializeMessage(QByteArray message);

    QByteArray serializeMessage();
};
