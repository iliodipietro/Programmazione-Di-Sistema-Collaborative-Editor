
#pragma once
#include "Serialize.h"
#include <QtWidgets/QWidget>
//#include "ui_Serialize.h"
#include "CRDT/Message.h"
#include "define.h"
#include "Editor/CustomCursor.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QPixMap>
#include <string>
#include <vector>
#include <QByteArray>

class Serialize : public QObject
{
	Q_OBJECT

public:

	Serialize(QObject* parent = Q_NULLPTR);

	/*-------------------------------------------------------------------------------------------------------------
															USO GENERALE
	quando arriva dal socket un QByteArray si usa la funzione fromArrayToObject per ottenere il QJson Object corrispondente.
	con tale oggetto si chiama la funzione actionType che mi dice il tipo di messaggio--> sono le define da decidere
	in base a tale valore si puo capire quale dei deserializzatori usare con uno switch
	--------------------------------------------------------------------------------------------------------------*/

	//QJsonObject unserialize(QString str); // old
	static int actionType(QJsonObject obj);


	static QJsonObject userSerialize(QString user, QString password, QString nickname, int type, QPixmap* profileImage = Q_NULLPTR);//type usato per discriminare login o register
	static QStringList userUnserialize(QJsonObject obj);//in particolare la lista contiene 2 elementi se uso login oppure 3 se uso
	//la register l'immagine viene serializzata a parte per ora



	static QJsonObject fileNameSerialize(QString fileName, int type);

	static QString fileNameUnserialize(QJsonObject obj);

	static QJsonObject FileListSerialize(QMap<int, QString> files, int type);// ilio
	static QMap<int, QString> fileListUnserialize(QJsonObject obj);// ilio


	static QJsonObject newFileSerialize(QString filename, int type);// ilio
	static QPair<int,QString> newFileUnserialize(QJsonObject obj);// ilio


	static QJsonObject messageSerialize(Message message, int type);//qui abbiamo sia il messaggio con all'interno un simbolo
	static Message messageUnserialize(QJsonObject obj);



	static QJsonObject textMessageSerialize(QString str, int type);
	static QString textMessageUnserialize(QJsonObject obj);

	static QJsonObject imageSerialize(QPixmap img, int type);
	static QPixmap imageUnserialize(QJsonObject obj);

	static QJsonObject responseSerialize(bool res, QString message, int userID, int type);
	static QStringList responseUnserialize(QJsonObject obj);

	static QJsonObject ObjectFromString(QString& in);

	static QJsonObject cursorPostionSerialize(int position, int user, int type);
	static std::vector<int> cursorPostionUnserialize(QJsonObject obj);

	static QJsonObject cursorSerialize(CustomCursor cursor, int type);
	static CustomCursor cursorUnserialize(QJsonObject obj);

	static QByteArray fromObjectToArray(QJsonObject obj);
	static QJsonObject fromArrayToObject(QByteArray data);


	//void setType(QString type);


private:
	//Ui::SerializeClass ui;
	//che tipo di oggetto voglio usare viene incapsulato nel json--> login,register,ecc
	//QString type;
	static QJsonValue jsonValFromPixmap(const QPixmap& p);
	static QPixmap pixmapFrom(const QJsonValue& val);

};