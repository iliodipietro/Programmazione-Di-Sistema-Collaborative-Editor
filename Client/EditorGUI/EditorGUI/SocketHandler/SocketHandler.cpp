#include "SocketHandler.h"
#include <QtCore\qjsondocument.h>

#define SERVER_IP "192.168.0.6"

SocketHandler::SocketHandler(QObject* parent) : QObject(parent), m_tcpSocket(QSharedPointer<QTcpSocket>(new QTcpSocket(this))),
m_messageSerializer(QSharedPointer<Serialize>(new Serialize(this)))
{
	m_tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
	connect(m_tcpSocket.get(), SIGNAL(connected()), this, SLOT(connected()));
	connect(m_tcpSocket.get(), SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(m_tcpSocket.get(), SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(m_tcpSocket.get(), SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
	//connect(this, SIGNAL(dataReceived(int)), this, SLOT(getMessage(int)));
}

bool SocketHandler::connectToServer() {
	m_tcpSocket->bind(QHostAddress(SERVER_IP), 0);
	if (!m_tcpSocket->waitForDisconnected(1000))
	{
		qDebug() << "Error: " << m_tcpSocket->errorString();
		return false;
	}
	return true;
}

void SocketHandler::connected()
{
	qDebug() << "Connected!";
	QByteArray data = QString("connesso").toUtf8();
	m_tcpSocket->write(intToArray(data.size()));
	m_tcpSocket->write(data);
	m_tcpSocket->waitForBytesWritten(3000);
}

void SocketHandler::disconnected()
{
	qDebug() << "Disconnected!";
}

void SocketHandler::bytesWritten(qint64 bytes)
{
	qDebug() << "We wrote: " << bytes;
}

void SocketHandler::readyRead()
{
	while (m_tcpSocket->bytesAvailable()) {
		qint64 numBytes = m_tcpSocket->bytesAvailable();
		QByteArray data = m_tcpSocket->readAll();
		while (data.size() >= 8) {

			qint64 messageSize = arrayToInt(data.mid(0, 8));

			data.remove(0, 8);

			if (messageSize > 0 && data.size() >= messageSize) {
				QByteArray message = data.mid(0, static_cast<qint32>(messageSize));
				data.remove(0, static_cast<qint32>(messageSize));
				QJsonParseError parseError;
				QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);
				emit dataReceived(doc.object());
			}
		}

		/*qDebug() << "Reading...";
		qDebug() << m_tcpSocket->readAll();*/
	}
}

QString responseForSynchronousWait(int message) {
	return message;
}

bool SocketHandler::writeData(SocketMessage& data) {
	if (m_tcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		QByteArray bytes = data.serializeMessage();
		m_tcpSocket->write(intToArray(bytes.size()));
		m_tcpSocket->write(bytes);
		return m_tcpSocket->waitForBytesWritten();

	}
	else {
		return false;
	}
}

bool SocketHandler::writeData(QByteArray& data) {
	if (m_tcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		m_tcpSocket->write(intToArray(data.size()));
		m_tcpSocket->write(data);
		return m_tcpSocket->waitForBytesWritten();
		readyRead();
	}
	else {
		return false;
	}
}

QByteArray SocketHandler::intToArray(qint32 source) {
	QByteArray temp;
	QDataStream data(&temp, QIODevice::ReadWrite);
	data << source;
	return temp;
}

qint64 SocketHandler::arrayToInt(QByteArray source)
{
	qint64 temp;
	QDataStream data(&source, QIODevice::ReadWrite);
	data >> temp;
	return temp;
}

QAbstractSocket::SocketState SocketHandler::getSocketState() {
	return m_tcpSocket->state();
}
