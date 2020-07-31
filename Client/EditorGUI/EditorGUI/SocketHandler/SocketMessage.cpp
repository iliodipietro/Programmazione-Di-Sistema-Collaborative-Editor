#include "SocketMessage.h"

//QByteArray intToArray(qint32 source);
//qint32 arrayToInt(QByteArray source);

SocketMessage::SocketMessage(QByteArray message) {
	qint32 type = arrayToInt(message.mid(0, 4));
	m_messageType = static_cast<MessageTypes>(type);
	message.remove(0, 4);
	m_message = message;
}

//QSharedPointer<SocketMessage> SocketMessage::deserializeMessage(QByteArray message) {
//	
//	qint32 type = arrayToInt(message.mid(0, 4));
//	message.remove(0, 4);
//	QSharedPointer<SocketMessage> m = QSharedPointer<SocketMessage>(new SocketMessage(static_cast<MessageTypes>(type), message));
//	return m;
//}

QByteArray SocketMessage::serializeMessage() {
	QByteArray data;
	//data.append(intToArray(m_size));
	data.append(intToArray(m_messageType));
	data.append(m_message);

	return data;
}

QByteArray SocketMessage::intToArray(qint32 source) {
	QByteArray temp;
	QDataStream data(&temp, QIODevice::ReadWrite);
	data << qint8(source);
	/*while (temp.size() > 1 && temp[0] == 0)
		temp.remove(0, 1);*/
	return temp;
}

qint32 SocketMessage::arrayToInt(QByteArray source)
{
	qint32 temp;
	QDataStream data(&source, QIODevice::ReadWrite);
	data >> temp;
	return temp;
}
