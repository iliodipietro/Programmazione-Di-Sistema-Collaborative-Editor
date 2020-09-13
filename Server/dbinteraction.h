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
    static void sendMessage(QTcpSocket *socket, QByteArray obj);

    void registration(QString username, QString password, QString nickname, QString profileImage, QTcpSocket *socket);
    void login(QString username, QString password, ClientManager *socket);

    void createFile(QString filename, QString username, QTcpSocket *socket);
    void openFile(int fileId, QString username, QTcpSocket *socket);
    void closeFile(int fileId, QString username, QTcpSocket *socket);
    void deleteFile();
    void renameFile();


    void forwardMessage(ClientManager* user, QJsonObject obj, QByteArray data);

    File* getFile(int fileid);

    void funzionedaeliminare() {
        File* primo_file = new File(0, "C:/Users/Mattia Proietto/Desktop/PROVAEBASTA.txt");
        instance->files.insert(0, primo_file);
    }
private:
    static DBInteraction* instance;
    QSqlDatabase db;

    //QMap<ClientManager*, int> users;
    QMap<QString, ClientManager*>users;
    QMap<int, File*> files; // mappa fileId - File

    QByteArray intToArray(qint64 source);


};

#endif // DBINTERACTION_H
