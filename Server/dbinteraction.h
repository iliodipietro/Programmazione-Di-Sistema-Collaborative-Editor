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

class Serialize;

class DBInteraction
{
public:
    DBInteraction();
    static DBInteraction *startDBConnection();
    static QString generateRandomString(int len);
    static void sendMessage(QTcpSocket *socket, QByteArray obj);

    void registration(QString username, QString password, QTcpSocket *socket);
    void login(QString username, QString password, QTcpSocket *socket);

    void createFile(QString filename, QString username, QTcpSocket *socket);
    void openFile(int fileId, QString username, QTcpSocket *socket);
    void closeFile(int fileId, QString username, QTcpSocket *socket);
    void searchFile(); //?
    void deleteFile();
    void renameFile();

private:
    static DBInteraction* instance;
    QSqlDatabase db;
    QMap<QString, QMap<int, QString>> user_files;
    QSet<QString> users;
    QMap<int, QString> files;

};

#endif // DBINTERACTION_H
