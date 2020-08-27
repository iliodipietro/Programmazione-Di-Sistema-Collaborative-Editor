#ifndef FILE_H
#define FILE_H

#include <QObject>
#include "CRDT/CRDT.h"
#include <QTcpSocket>
#include "Serialize/Serialize.h"

class File
{
public:
    File();
    File(int fileId, QString path);
    void messageHandler(QTcpSocket* sender, Message m, QByteArray bytes);
    void addUser(QTcpSocket* user);
    void removeUser(QTcpSocket* user);
    QVector<QTcpSocket*> getUsers();
    bool thereAreUsers();
    void sendNewFile(QTcpSocket* socket);

private:
    CRDT *handler = nullptr;
    int id;
    //QMap<int,QTcpSocket*> owners;
    QString path;
    QVector<QTcpSocket*> users;
    void writeData(QTcpSocket* scoket, QByteArray bytes);

};

#endif // FILE_H
