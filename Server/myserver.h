#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "Serialize/Serialize.h"
#include "Serialize/define.h"

class MyServer : public QObject{
    Q_OBJECT
public:
    MyServer(QObject *parent = nullptr);
    bool listen(QHostAddress &addr, quint16 port);
    ~MyServer();

private slots:
    void onNewConnection();
    void readFromSocket();
    void MessageHandler(QTcpSocket *socket, QByteArray socketData);
    void onDisconnect();
signals:
    void bufferReady(QTcpSocket *socket, QByteArray socketData);


private:
    QTcpServer *_server = nullptr;

};

#endif // MYSERVER_H
