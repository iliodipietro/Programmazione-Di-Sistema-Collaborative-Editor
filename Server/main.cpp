#include <QCoreApplication>
#include "myserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyServer s;

    /*
    QMap<int, QString> prova;
    QMap<int, QString> res;
    QJsonObject obj;

    prova.insert(1, "ciao");
    prova.insert(2, "albero");

    obj =  Serialize::FileListSerialize(prova, SEND_FILES);
    res = Serialize::fileListUnserialize(obj);

    qDebug()<< res;
    */

    return a.exec();
}
//prova di commit a cazzo
