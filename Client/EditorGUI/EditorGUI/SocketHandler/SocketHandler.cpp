#include "SocketHandler.h"
#include <QtCore\qjsondocument.h>
#include <QDebug>
#include <QDir>

//#define SERVER_IP "127.0.0.1"
//#define PORT 44322

SocketHandler::SocketHandler(QObject* parent) : QObject(parent), m_tcpSocket(QSharedPointer<QTcpSocket>(new QTcpSocket(this))),
m_previousPacket(QSharedPointer<QByteArray>(new QByteArray())), m_previousSize(0)
{
	m_tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
	//m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	m_tcpSocket->setReadBufferSize(0);
	connect(m_tcpSocket.get(), SIGNAL(connected()), this, SLOT(connected()));
	connect(m_tcpSocket.get(), SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(m_tcpSocket.get(), SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(m_tcpSocket.get(), SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
	//connect(this, SIGNAL(dataReceived(int)), this, SLOT(getMessage(int)));
	readConfigFile();
	connectToServer();
}

bool SocketHandler::connectToServer() {
	m_tcpSocket->connectToHost(QHostAddress(m_serverIp), m_serverPort);
	/*if (!m_tcpSocket->waitForDisconnected(1000))
	{
		qDebug() << "Error: " << m_tcpSocket->errorString();
		return false;
	}*/
	return true;
}

void SocketHandler::connected()
{
	qDebug() << "Connected!";
	QByteArray data = QString("connesso").toUtf8();
	m_tcpSocket->write(intToArray(data.size()).append(data));
	qDebug() << data.size() << "\n";
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
	while (m_tcpSocket->bytesAvailable() || m_previousPacket->size() != 0) {
		qint64 numBytes = m_tcpSocket->bytesAvailable();
		QByteArray data = m_tcpSocket->readAll();
		m_previousPacket->append(data);
		while ((m_previousSize == 0 && m_previousPacket->size() >= 8) || (m_previousSize > 0 && m_previousPacket->size() >= m_previousSize)) {

			if (m_previousSize == 0 && m_previousPacket->size() >= 8) {
				m_previousSize = arrayToInt(m_previousPacket->mid(0, 8));
				m_previousPacket->remove(0, 8);
			}

			if (m_previousSize > 0 && m_previousPacket->size() >= m_previousSize) {
				QByteArray message = m_previousPacket->mid(0, m_previousSize);
				m_previousPacket->remove(0, m_previousSize);
				m_previousSize = 0;
				QJsonParseError parseError;
				QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);
				emit dataReceived(doc.object());
			}
		}

		if (m_previousPacket->size() < 8 || (m_previousSize > 0 && m_previousPacket->size() < m_previousSize)) break;

		/*qDebug() << "Reading...";
		qDebug() << m_tcpSocket->readAll();*/
	}
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
		m_tcpSocket->write(intToArray(data.size()).append(data));
		return m_tcpSocket->waitForBytesWritten();
	}
	else {
		return false;
	}
}

QByteArray SocketHandler::intToArray(qint64 source) {
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

void SocketHandler::readConfigFile() {
	QString path = QDir::currentPath().append("/config.txt");
	QFile file(path);
	QString line;
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		line = stream.readLine();
		m_serverIp = line.section(":", 1, 1);
		line = stream.readLine();
		m_serverPort = line.section(":", 1, 1).toInt();
		file.close();
	}
}
