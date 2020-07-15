#include "Serialize.h"
#include <QBuffer>
#include <QJsonDocument>

Serialize::Serialize(QWidget *parent)
	: QMainWindow(parent)
{
	//ui.setupUi(this);
}


QString Serialize::userSerialize(QString user, QString password,QString nickname, int type)
{
	QJsonObject obj;
	
	obj.insert("type", QJsonValue(type));//??
	obj.insert("user",QJsonValue(user));
	obj.insert("password", QJsonValue(password));

	if (type == REGISTER) {
		obj.insert("nickname",QJsonValue(nickname));
	}

	QJsonDocument doc(obj);
	QString strJson(doc.toJson(QJsonDocument::Compact));

	return strJson.append("\r\n");
}

QStringList Serialize::userUnserialize(QJsonObject obj)
{

	QString usr = obj.value("user").toString();
	QString password = obj.value("password").toString();
	//QString type = obj.value("type").toString();
	
	QStringList list;
	list.append(usr);
	list.append(password);
	if (this->actionType(obj)==REGISTER) {
		QString nickname = obj.value("nickname").toString();
		list.append(nickname);
	}
	
	return list;
}




QString Serialize::fileNameSerialize(QString fileName, int type)
{

	QJsonObject obj;
	obj.insert("type", QJsonValue(type));//??
	obj.insert("filename",fileName);

	QJsonDocument doc(obj);
	QString strJson(doc.toJson(QJsonDocument::Compact));
	return strJson.append("\r\n");
}

QString Serialize::fileNameUnserialize(QJsonObject obj)
{

	return obj.value("filename").toString();
}





QString Serialize::messageSerialize(Message message, int type)
{
	QJsonObject obj;
	
	obj.insert("type", QJsonValue(type));//??

	int action = message.getAction();
	
	int senderId = message.getSenderId();
	
	obj.insert("action",QJsonValue(action));
	obj.insert("sender",QJsonValue(senderId));


	Symbol s = message.getSymbol();

	char c = s.getChar();

	obj.insert("character", QJsonValue(c));

	//include di vector e array sono gia in symbol includso in message
	std::array<int, 2> id = s.getId();

	QJsonArray  Qid = {id[0],id[1]};//id globale

	obj.insert("globalCharacterId", QJsonValue(Qid));

	std::vector<int> v = s.getPos();

	QJsonArray Qvett;

	for (int i : v) {
		
		Qvett.append(QJsonValue(i));
	}

	obj.insert("position",QJsonValue(Qvett));


	//font e colore
	QFont font = s.getFont();
	QString serialFont = font.toString();
	obj.insert("font",QJsonValue(serialFont));

	QString color = s.getColor().name();
	Qt::AlignmentFlag aligment = s.getAlignment();
	
	//int red = color.red();
	//int green = color.green();
	//int blue = color.blue();

	//obj.insert("red",QJsonValue(red));
	//obj.insert("green", QJsonValue(green));
	//obj.insert("blue", QJsonValue(blue));
	obj.insert("color", color);
	obj.insert("aligment", aligment);


	QJsonDocument doc(obj);
	QString strJson(doc.toJson(QJsonDocument::Compact));
	return strJson.append("\r\n");
}

Message Serialize::messageUnserialize(QJsonObject obj)
{

	char c = obj.value("character").toInt();

	std::array<int, 2> a;
	QJsonArray id = obj.value("globalCharacterId").toArray();
	a[0] = id[0].toInt();
	a[1] = id[1].toInt();

	std::vector<int> pos;

	QJsonArray vett_pos = obj.value("position").toArray();

	for (QJsonValue qj : vett_pos) {
	
		pos.push_back(qj.toInt());
	}

	QFont font;
	font.fromString(obj.value("font").toString());

	//int red = obj.value("red").toInt();
	//int green = obj.value("green").toInt();
	//int blue = obj.value("blue").toInt();
	QString color_hex = obj.value("color").toString();

	QColor color(color_hex);

	//FINIRE QUESTOOOOOOOOOOOO
	int align = obj.value("alignment").toInt();
	Qt::AlignmentFlag alignFlag = static_cast<Qt::AlignmentFlag>(align);

	
	Symbol s(c , a , pos,font,color, alignFlag);
	
	int action = obj.value("action").toInt();

	int sender = obj.value("sender").toInt();
	
	Message m(s,action,sender);

	return m;
}

QString Serialize::textMessageSerialize(QString str, int type)
{

	QJsonObject obj;

	obj.insert("type", QJsonValue(type));//??
	obj.insert("message", QJsonValue(str));

	QJsonDocument doc(obj);
	QString strJson(doc.toJson(QJsonDocument::Compact));
	return strJson.append("\r\n");
}

QString Serialize::textMessageUnserialize(QJsonObject obj)
{

	return obj.value("message").toString();
}


QJsonValue  Serialize::jsonValFromPixmap(const QPixmap &p) {
	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	p.save(&buffer, "PNG");
	auto const encoded = buffer.data().toBase64();
	return { QLatin1String(encoded) };
}

QPixmap  Serialize::pixmapFrom(const QJsonValue &val) {
	auto const encoded = val.toString().toLatin1();
	QPixmap p;
	p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
	return p;
}

QString Serialize::imageSerialize(QPixmap img, int type)
{
	QJsonObject obj;

	obj.insert("type", QJsonValue(type));//??

	obj.insert("img",this->jsonValFromPixmap(img));
	
	QJsonDocument doc(obj);
	QString strJson(doc.toJson(QJsonDocument::Compact));
	return strJson.append("\r\n");
}

QPixmap Serialize::imageUnserialize(QJsonObject obj)
{
	QPixmap map;

	map = this->pixmapFrom(obj.value("img"));

	return map;
}

QString Serialize::responseSerialize(int res, int type)
{
	QJsonObject obj;

	obj.insert("type", QJsonValue(type));//??

	obj.insert("res", QJsonValue(res));

	QJsonDocument doc(obj);
	QString strJson(doc.toJson(QJsonDocument::Compact));
	return strJson.append("\r\n");
}

int Serialize::responseUnserialize(QJsonObject obj)
{
	return obj.value("res").toInt();
}


QJsonObject Serialize::unserialize(QString str)
{
	QJsonDocument doc = QJsonDocument::fromJson(str.toLatin1());

	QJsonObject obj = doc.object();

	return obj;
}

int Serialize::actionType(QJsonObject obj)
{
	return obj.value("type").toInt();
}

//void Serialize::setType(QString type)
//{
//	this->type = type;
//}
