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
#include "dbinteraction.h"
#include "CRDT/CRDT.h"
#include "CRDT/Message.h"
#include "CRDT/Symbol.h"
class CRDT;
class Serialize;
class QTcpServer;

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
    void dataReady(QTcpSocket *socket, QByteArray socketData);


private:
    QTcpServer *_server = nullptr;
    QMap <QTcpSocket*, QByteArray> socket_buffer;
    DBInteraction *db = nullptr;

};

#endif // MYSERVER_H
