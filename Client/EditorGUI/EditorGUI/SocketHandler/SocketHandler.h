#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QCloseEvent>
#include <QSharedPointer>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include "Serialization/Serialize.h"

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    SocketHandler(QObject* parent = Q_NULLPTR);

    bool writeData(QByteArray& data);

    QAbstractSocket::SocketState getSocketState();

    ~SocketHandler();

private:
    QSharedPointer<QByteArray> m_previousPacket;
    QSharedPointer<QTcpSocket> m_tcpSocket;
    QString m_serverIp;
    int m_serverPort;
    qint64 m_previousSize;
    bool m_readThreadRun;
    std::atomic_bool m_continueReading;
    std::thread* m_readThread;
    std::mutex m_readBufferMutex;
    std::condition_variable m_readBufferCV;
    std::vector<QJsonObject> m_packetsInQueue;

    void readConfigFile();
    bool connectToServer();
    void closeEvent(QCloseEvent* event);
    QByteArray intToArray(qint64 source);
    qint64 arrayToInt(QByteArray source);
    void readThreadFunction();
    void parseEmitMessages(QByteArray* message);

signals:
    void dataReceived(QJsonObject);

public slots:

    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
};
