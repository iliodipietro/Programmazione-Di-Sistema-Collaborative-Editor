#include "SocketMessage.h"

QSharedPointer<SocketMessage> SocketMessage::deserializeMessage(QByteArray message){
    qint32 type = arrayToInt(message.mid(0, 4));
    message.remove(0, 4);
    QSharedPointer<SocketMessage> m = QSharedPointer<SocketMessage>(new SocketMessage(static_cast<MessageTypes>(type), message));
    return m;
}

QByteArray SocketMessage::serializeMessage(){
    QByteArray data;
    //data.append(intToArray(m_size));
    data.append(intToArray(m_messageType));
    data.append(m_message);

    return data;
}

QByteArray SocketMessage::intToArray(qint32 source) {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

qint32 SocketMessage::arrayToInt(QByteArray source)
{
    qint32 temp;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> temp;
    return temp;
}
