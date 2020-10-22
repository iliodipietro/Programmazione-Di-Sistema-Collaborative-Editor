
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

	static QJsonObject changeProfileSerialize(QString oldUsername, QString newUsername, QString oldEmail, QString newEmail, QPixmap* newImage, int type);

	static QStringList changeProfileUnserialize(QJsonObject obj);


	static QJsonObject userSerialize(QString user, QString password, QString email, int type, QPixmap* profileImage = Q_NULLPTR);//type usato per discriminare login o register
	static QStringList userUnserialize(QJsonObject obj);//in particolare la lista contiene 2 elementi se uso login oppure 3 se uso
	//la register l'immagine viene serializzata a parte per ora



	static QJsonObject fileNameSerialize(QString fileName, int type);

	static QString fileNameUnserialize(QJsonObject obj);

	static QJsonObject FileListSerialize(QMap<int, QString> files, int type);// ilio
	static QMap<int, QString> fileListUnserialize(QJsonObject obj);// ilio


	static QJsonObject newFileSerialize(QString filename, int type);// ilio
	static QPair<int,QString> newFileUnserialize(QJsonObject obj);// ilio

	static QJsonObject openDeleteFileSerialize(int fileId, int type); // ilio
	static int openDeleteFileUnserialize(QJsonObject obj);// ilio

	static QJsonObject closeFileSerialize(int fileId, int siteCounter, int type);
	static QPair<int, int> closeFileUnserialize(QJsonObject obj);

	static QJsonObject renameFileSerialize(int fileId, QString newName, int type); // ilio
	static QStringList renameFileUnserialize(QJsonObject obj);

	static QJsonObject openSharedFileSerialize(QString URI, int type); // ilio
	static QString openSharedFileUnserialize(QJsonObject obj); // ilio

	static QStringList changeProfileResponseUnserialize(QJsonObject obj); //ilio

	static QJsonObject messageSerialize(Message message, int fileId, int type);//qui abbiamo sia il messaggio con all'interno un simbolo
	static QPair<int, Message> messageUnserialize(QJsonObject obj);

	static QJsonObject changePasswordSerialize(QString oldPassword, QString newPassword, int type); //lorenzo
	static QStringList changePasswordUnserialize(QJsonObject obj); //lorenzo

	static QJsonObject textMessageSerialize(QString str, int type);
	static QString textMessageUnserialize(QJsonObject obj);

	static QJsonObject imageSerialize(QPixmap img, int type);
	static QPixmap imageUnserialize(QJsonObject obj);

	static QJsonObject responseSerialize(bool res, QString message, int type, QString username = "", QString email = "", int userID = -1, QColor color = Qt::black);
	static QStringList responseUnserialize(QJsonObject obj);

	static QString URIUnserialize(QJsonObject uri);

	static QJsonObject ObjectFromString(QString& in);

	static QJsonObject cursorPostionSerialize(int position, int user, int fileId, int type);
	static std::vector<int> cursorPostionUnserialize(QJsonObject obj);

	static QJsonObject cursorSerialize(CustomCursor cursor, int type);
	static CustomCursor cursorUnserialize(QJsonObject obj);

	static QJsonObject addEditingUserSerialize(int userId, QString username, QColor userColor, int fileId, int type); //augusto
	static QPair<int, QStringList> addEditingUserUnserialize(QJsonObject obj);//augusto

	static QJsonObject removeEditingUserSerialize(int userId, int fileId, int type);//augusto
	static QPair<int, int> removeEditingUserUnserialize(QJsonObject obj);//augusto

	static QJsonObject requestFileList(int type); //augusto

	static QJsonObject logoutUserSerialize(int type);//augusto

	static QByteArray fromObjectToArray(QJsonObject obj);
	static QJsonObject fromArrayToObject(QByteArray data);

	static QPair<int, int> Serialize::siteCounterUnserialize(QJsonObject obj);
	//static QJsonObject openDeleteFileSerialize(int fileId, int type); //lorenzo, per la condivisione file


	//void setType(QString type);


private:
	//Ui::SerializeClass ui;
	//che tipo di oggetto voglio usare viene incapsulato nel json--> login,register,ecc
	//QString type;
	static QJsonValue jsonValFromPixmap(const QPixmap& p);
	static QPixmap pixmapFrom(const QJsonValue& val);

};