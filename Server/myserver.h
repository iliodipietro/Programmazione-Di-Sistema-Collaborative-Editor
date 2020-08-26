#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
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
    QMap <QTcpSocket*, QPair<QByteArray*, quint32>> socket_buffer;//We need a buffer to store data until block has completely received.
                                                                 //The int represent the dimension of the data receiver and it is sent as first parameter in the socket
    DBInteraction *db = nullptr;

    ///MATTIA--------------------------------

    std::map<int, CRDT*> fileId_CRDT;//mi serve un crdt per ogni file 

    void handleMessage(int fileID, Message m);
    std::vector<Message> readFileFromDisk(std::string path, int fileID);
    void sendNewFile(std::vector<Message> messages ,int fileId);

    bool addFile(int fileID, std::string path);//false se file gi� presente o errore
    void removeFile(int fileID);// se lo trova elimina altrimenti non fa nulla


    //-------------------------------------
};

#endif // MYSERVER_H
