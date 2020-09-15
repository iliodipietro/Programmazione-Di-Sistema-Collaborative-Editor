#ifndef FILE_H
#define FILE_H

#include <QObject>
#include "CRDT/CRDT.h"
#include <QTcpSocket>
#include "Serialize/Serialize.h"
#include "ClientManager/clientmanager.h"
class File
{
public:
    File();
    File(int fileId, QString path);
    void messageHandler(ClientManager* sender, Message m, QByteArray bytes);
    void addUser(ClientManager* user);
    void removeUser(ClientManager* user);

    QList<ClientManager*> getUsers();
    bool thereAreUsers();
    void sendNewFile(ClientManager* socket);
    void updateCursorPosition(ClientManager* sender, QByteArray message);

private:
    CRDT *handler = nullptr;
    int id;
    //QMap<int,QTcpSocket*> owners;
    QString path;
    //QVector<ClientManager*> users;
    QMap <int, ClientManager*> users;


};

#endif // FILE_H
