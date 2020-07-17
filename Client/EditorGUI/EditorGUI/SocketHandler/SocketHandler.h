#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QCloseEvent>
#include <QSharedPointer>
#include "SocketMessage.h"

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit SocketHandler(QObject *parent = nullptr);

    bool connectToServer();
    bool writeData(SocketMessage& data);

private:
    QSharedPointer<QTcpSocket> m_tcpSocket;
    QSharedPointer<QNetworkSession> m_networkSession;

    void closeEvent(QCloseEvent* event);
    QByteArray intToArray(qint32 source);
    qint64 arrayToInt(QByteArray source);

signals:
    void dataReceived(QSharedPointer<SocketMessage>);

public slots:

    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
};
