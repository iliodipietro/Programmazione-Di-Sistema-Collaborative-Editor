#ifndef FILE_H
#define FILE_H

#include <QObject>
#include "CRDT/CRDT.h"
#include <QTcpSocket>

class File
{
public:
    File();



private:
    CRDT *handler;
    int id;
    QMap<QString, QTcpSocket*> owners;

};

#endif // FILE_H
