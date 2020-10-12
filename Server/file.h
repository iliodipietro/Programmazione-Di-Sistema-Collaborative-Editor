#ifndef FILE_H
#define FILE_H

#include <QObject>
#include "CRDT/CRDT.h"
#include <QTcpSocket>
#include "Serialize/Serialize.h"
#include "ClientManager/clientmanager.h"

struct CursorPosition{
    std::vector<int> pos;
    bool afterInsert;

    CursorPosition(){};
    CursorPosition(std::vector<int> pos, bool afterInsert) : pos(pos), afterInsert(afterInsert){};
};

class File
{
public:
    File();
    ~File();
    File(int fileId, QString path);
    void messageHandler(ClientManager* sender, Message m, QByteArray bytes);
    void addUser(ClientManager* user);
    void removeUser(ClientManager* user);

    QList<ClientManager*> getUsers();
    bool thereAreUsers();
    void sendNewFile(ClientManager* socket);
    bool isModifiedName();
    QString getNewName();
    QString getPath();
    void modifyName(QString newName);
    void updateCursorPosition(ClientManager* sender, QByteArray message);

private:
    CRDT *handler = nullptr;
    int id;
    bool modifiedName;
    QString newName;
    //QMap<int,QTcpSocket*> owners;
    QString path;
    //QVector<ClientManager*> users;
    QMap <int, ClientManager*> users;
    QMap <ClientManager*, CursorPosition> m_usersCursorPosition;

};

#endif // FILE_H
