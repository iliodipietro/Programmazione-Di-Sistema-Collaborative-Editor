#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QCloseEvent>
#include <QSharedPointer>
#include <thread>
#include "Serialization/Serialize.h"

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    SocketHandler(QObject* parent = Q_NULLPTR);

    void run();

    QAbstractSocket::SocketState getSocketState();

    ~SocketHandler();

private:
    QByteArray* m_previousPacket;
    QTcpSocket* m_tcpSocket;
    QString m_serverIp;
    int m_serverPort;
    qint32 m_previousSize;
    std::vector<QJsonObject> m_packetsInQueue;

    void readConfigFile();
    bool connectToServer();
    void closeEvent(QCloseEvent* event);
    QByteArray intToArray(qint32 source);
    qint32 arrayToInt(QByteArray source);
    void readThreadFunction();

signals:
    void dataReceived(QJsonObject);

public slots:

    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
    bool writeData(QByteArray data);
};
