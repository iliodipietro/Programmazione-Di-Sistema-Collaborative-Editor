#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QCloseEvent>
#include <QSharedPointer>
#include "SocketMessage.h"
#include "Serialization/Serialize.h"

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    SocketHandler(QObject *parent = Q_NULLPTR);

    bool connectToServer();
    bool writeData(SocketMessage& data);
    bool writeData(QByteArray& data);
    QString writeDataAndWaitForResponse(QString data);

    QAbstractSocket::SocketState getSocketState();

private:
    QSharedPointer<QTcpSocket> m_tcpSocket;
    QSharedPointer<QNetworkSession> m_networkSession;
    QSharedPointer<Serialize> m_messageSerializer;

    void closeEvent(QCloseEvent* event);
    QByteArray intToArray(qint32 source);
    qint64 arrayToInt(QByteArray source);

signals:
    void dataReceived(QJsonObject);

public slots:

    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
};
