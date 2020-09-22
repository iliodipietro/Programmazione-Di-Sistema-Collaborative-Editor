#ifndef DBINTERACTION_H
#define DBINTERACTION_H


#include <cstdlib>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QMap>
#include <QSet>
#include <QString>
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include "Serialize/Serialize.h"
#include "Serialize/define.h"
#include "file.h"

class Serialize;
class File;

class DBInteraction
{
public:
    DBInteraction();
    static DBInteraction *startDBConnection();

    static QString generateRandomString(int len);
   // static void sendMessage(QTcpSocket *socket, QByteArray obj);
    static void sendError(ClientManager* user);
    static void sendSuccess(ClientManager* user);
    static QString computeHashPassword(QString password);
    static bool checkPassword(QString password, ClientManager* client);

    void registration(QString username, QString password, QString nickname, QString profileImage, ClientManager* incomingClient);
    void login(QString username, QString password,  ClientManager* incomingClient);
    void logout(ClientManager* user);

    void createFile(QString filename, ClientManager* client);
    void sendFileList(ClientManager* client);
    void openFile(int fileId, ClientManager* client, QString URI = nullptr);
    void closeFile(int fileId, int siteCounter, ClientManager* client);
    void deleteFile(int fileId, ClientManager* client);
    void renameFile(int fileId, QString newname, ClientManager* client);
    void getURIToShare(int fileid, ClientManager* client);
    void openSharedFile(QString URI, ClientManager* client);

    void changePassword(QString oldPassword, QString newPassword, ClientManager* client);
    void changeUsername(QString newUsername, ClientManager* client);
    void changeNickname(QString newNickname, ClientManager* client);
    void changeProfilePic(QString profileImage, ClientManager* client);
    void changeProfile(QString newUsername, QString newNick, QString newImagePath, ClientManager *client);


    void forwardMessage(ClientManager* user, QJsonObject obj, QByteArray data);

    File* getFile(int fileid);
    bool isUserLogged(ClientManager* client);


    void funzionedaeliminare() {
        File* primo_file = new File(-1, "C:/Users/Mattia Proietto/Desktop/PROVAEBASTA.txt");
        instance->files.insert(-1, primo_file);
    }
private:
    static DBInteraction* instance;
    QSqlDatabase db;

    QVector<ClientManager*> activeusers;
    QMap<int, File*> files; // mappa fileId - File contenente tutti i file attivi al momentento

    QByteArray intToArray(qint64 source);


};

#endif // DBINTERACTION_H
