#pragma once

#include "CRDT/Message.h"
#include "define.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QImage>
#include <string>
#include <vector>
#include <QByteArray>
#include <QPixmap>

class Serialize
{
public:

    Serialize(QWidget* parent = Q_NULLPTR);

    /*-------------------------------------------------------------------------------------------------------------
                                                            USO GENERALE
    quando arriva dal socket un QByteArray si usa la funzione fromArrayToObject per ottenere il QJson Object corrispondente.
    con tale oggetto si chiama la funzione actionType che mi dice il tipo di messaggio--> sono le define da decidere
    in base a tale valore si puo capire quale dei deserializzatori usare con uno switch
    --------------------------------------------------------------------------------------------------------------*/

    //QJsonObject unserialize(QString str); // old
    static int actionType(QJsonObject obj);

    //usate in myserver.cpp per login o register
    static QJsonObject userSerialize(QString user, QString password, QString nickname, int type);//type usato per discriminare login o register
    static QStringList userUnserialize(QJsonObject obj);//in particolare la lista contiene 2 elementi se uso login oppure 3 se uso
    //la register l'immagine viene serializzata a parte per ora

    //usate in DBInteraction per login
    static QJsonArray singleFileSerialize(QString fileName, int fileId, QJsonArray files);//ilio
    static QJsonObject user_filesSerialize(int userId, QString username, QJsonArray files, int type);//ilio
    static QPair<int, QMap<int, QString>> user_filesUnserialize(QJsonObject obj);//ilio

    //usate in myserver.cpp per open - close
    static QJsonObject openCloseFileSerialize(int fileId, QString username, int type); // ilio
    static QPair<int, QString> openCloseFileUnserialize(QJsonObject obj);// ilio

    static QJsonObject newFileSerialize(QString filename, QString username, int type);//ilio
    static QPair<QString, QString> newFileUnserialize(QJsonObject obj);//ilio


    static QJsonObject messageSerialize(int fileId, Message message, int type);//qui abbiamo sia il messaggio con all'interno un simbolo
    static QPair<int, Message> messageUnserialize(QJsonObject obj);



    static QJsonObject textMessageSerialize(QString str, int type);
    static QString textMessageUnserialize(QJsonObject obj);

    static QJsonObject imageSerialize(QPixmap img, int type);
    static QPixmap imageUnserialize(QJsonObject obj);

    static QJsonObject responseSerialize(bool res, QString message, int type);
    static QStringList responseUnserialize(QJsonObject obj);

    static QJsonObject ObjectFromString(QString& in);

    static QJsonObject cursorPostionSerialize(int position, int user, int type);
    static std::vector<int> cursorPostionUnserialize(QJsonObject obj);

    static QByteArray fromObjectToArray(QJsonObject obj);
    static QJsonObject fromArrayToObject(QByteArray data);


    //void setType(QString type);


private:
    static QJsonValue jsonValFromPixmap(const QPixmap& p);
    static QPixmap pixmapFrom(const QJsonValue& val);

};
