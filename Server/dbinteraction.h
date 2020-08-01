#ifndef DBINTERACTION_H
#define DBINTERACTION_H


#include <cstdlib>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QTcpSocket>
#include <QCryptographicHash>
#include "Serialize/Serialize.h"
#include "Serialize/define.h"

class Serialize;

class DBInteraction
{
public:
    DBInteraction();
    static DBInteraction *startDBConnection();
    static QString generateRandomString(int len);
    void registration(QString username, QString password, QTcpSocket *socket);
    void login(QString username, QString password, QTcpSocket *socket);
    void createFile();
    void searchFile();

private:
    static DBInteraction* instance;
    QSqlDatabase db;

};

#endif // DBINTERACTION_H
