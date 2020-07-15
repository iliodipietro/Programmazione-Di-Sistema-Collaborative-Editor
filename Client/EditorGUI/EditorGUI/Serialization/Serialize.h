#pragma once

#include <QtWidgets/QMainWindow>
//#include "ui_Serialize.h"
#include "CRDT/Message.h"
#include "define.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QImage>
#include <string>

class Serialize : public QMainWindow
{
	Q_OBJECT

public:

	Serialize(QWidget *parent = Q_NULLPTR);

	/*-------------------------------------------------------------------------------------------------------------
	quando arriva dal socket una QString si usa unserialize per ottenere il QJson Object corrispondente,ogni Qstring 
	è qui terminata da un "/r/n" perchè potrebbe capitare arrivino più messaggi nella stessa read--> da gestire nel socket.
	con tale oggetto si chiama la funzione actionType che mi dice il tipo di messaggio--> sono le define da decidere
	in base a tale valore si puo capire quale dei deserializzatori usare con uno switch
	--------------------------------------------------------------------------------------------------------------*/

	QJsonObject unserialize(QString str);
	int actionType(QJsonObject obj);


	QString userSerialize(QString user, QString password,QString nickname,int type);//type usato per discriminare login o register
	QStringList userUnserialize(QJsonObject obj);//in particolare la lista contiene 2 elementi se uso login oppure 3 se uso
	//la eegister l'immagine viene serializzata a parte per ora

	
	
	QString fileNameSerialize(QString fileName, int type);

	QString fileNameUnserialize(QJsonObject obj);
	
	
	QString messageSerialize(Message message, int type);//qui abbiamo sia il messaggio con all'interno un simbolo
	Message messageUnserialize(QJsonObject obj);



	QString textMessageSerialize(QString str, int type);
	QString textMessageUnserialize(QJsonObject obj);

	QString imageSerialize(QPixmap img, int type);
	QPixmap imageUnserialize(QJsonObject obj);

	QString responseSerialize(int res,int type);
	int responseUnserialize(QJsonObject obj);



	//void setType(QString type);


private:
	//Ui::SerializeClass ui;
	//che tipo di oggetto voglio usare viene incapsulato nel json--> login,register,ecc
	//QString type;
	QJsonValue jsonValFromPixmap(const QPixmap &p);
	QPixmap pixmapFrom(const QJsonValue &val);	

};
