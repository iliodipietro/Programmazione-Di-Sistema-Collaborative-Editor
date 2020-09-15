#include "Serialize.h"
#include <QBuffer>
#include <QJsonDocument>
#include <QDebug>


Serialize::Serialize(QWidget* parent)
{
}



QJsonObject Serialize::userSerialize(QString user, QString password, QString nickname, QPixmap profileImage, int type)
{
    /*
    Questa funzione serializza i dati dell'utente quando vuole fare un login o signup, cio e' discriminato dal valore di type
    INPUT:
    - user: stringa che contiene lo username
    - password: stringa che contiene la password
    - nickname: stringa he contiene il nickname, questa stringa serve solo durante il sign-up viene ignorata se serializzo un messagio di tipo login
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, i tipi che possono esere passati qui sono LOGIN O REGISER(singup)
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson e terminata con \r\n-> vedere se serve effettivamente altrimenti eliminare
    */
    QJsonObject obj;

    obj.insert("type", QJsonValue(type));
    obj.insert("user", QJsonValue(user));
    obj.insert("password", QJsonValue(password));

    if (type == REGISTER) {
        //il nickname serve solo in fase di register per salvarlo sul server
        obj.insert("nickname", QJsonValue(nickname));
        obj.insert("img", Serialize::jsonValFromPixmap(profileImage));
    }


    //QJsonDocument doc(obj);
    //QString strJson(doc.toJson(QJsonDocument::Compact));

    //return strJson.append("\r\n");
    return obj;
}

QStringList Serialize::userUnserialize(QJsonObject obj)
{

    /*
    Questa funzione de-serializza i dati dell'utente quando vuole fare un login o signup, cio e' discriminato dal valore di type
    INPUT:
    - obj: e' un Qjson che contiene tutte le info dell'utente come username password e nickname per fare login o signup
    RETURN:
    - una QstringList che puo avere lunghezza 2 0 3.
    -->lunghezza 2 se LOGIN:
        list[0]: username
        list[1]: password
    -->lunghezza 3 se LOGIN:
        list[0]: username
        list[1]: password
        list[2]: nickname
    */

    QString usr = obj.value("user").toString();
    QString password = obj.value("password").toString();
    //QString type = obj.value("type").toString();

    QStringList list;
    list.append(usr);
    list.append(password);
    if (Serialize::actionType(obj) == REGISTER) {
        QString nickname = obj.value("nickname").toString();
        list.append(nickname);
        QString img = obj.value("img").toString();
        list.append(img);
        qDebug() << img;
    }

    return list;
}



QJsonObject Serialize::user_filesSerialize(int userId, QString username, QJsonArray files, int type){
    /*



QJsonObject Serialize::user_filesSerialize(int userId, QString username, QJsonArray files, int type){
    /*
    Questa funzione, una volta che tutti i file di un client sono stati correttamente serializzati nell'array files, lega l'utente ai file.
    INPUT:
    - username: stringa che contiene il nome dell'utente;
    - files: array serializzato contenente i campi(filename e id) per ogni file posseduto dal singolo client;
    - type: intero che è definito in define.h come LOGIN
    RETURN:
    - files: l'array che viene man mano aggiornato ad ogni chiamata.
    */

    QJsonObject obj;
    obj.insert("userid", userId);
    obj.insert("username", username);
    obj.insert("files", QJsonValue(files));
    obj.insert("type", type);

    return obj;
}

QPair<int, QMap<int, QString>> Serialize::user_filesUnserialize(QJsonObject obj){

    /*
    Questa funzione de-serializza il nome utente e tutti i file che possiede
    INPUT:
    - obj: e' un Qjson che contiene tutte le info
    RETURN:
    - una QPair<QString, QMap<int, QString>> con il nome dell'utente come primo elemento del QPair e come secondo una mappa contenente (fileId e FileName) di tutti i file dell'utente
    */

    QPair<int, QMap<int, QString>> user_files;
    QJsonArray files = obj.value("files").toArray();
    int i;
    QJsonObject singleFile;

    user_files.first = obj.value("username").toInt();

    for(i = 0; i < files.size(); i++){
        singleFile = files[i].toObject();
        user_files.second.insert(singleFile["fileId"].toInt(), singleFile["filename"].toString());
    }

    return user_files;
}


QJsonObject Serialize::openCloseFileSerialize(int fileId, QString username, int type)
{
    /*
    Questa funzione serializza l'id del file quando vuole fare un OPEN o CLOSE, cio e' discriminato dal valore di type
    INPUT:
    - fileId: intero che contiene l'id del file
    - username: stringa che rappresenta il possessore del file
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, i tipi che possono esere passati qui sono OPEN O CLOSE
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson
    */
    QJsonObject obj;
    obj.insert("type", QJsonValue(type));
    obj.insert("user", username);
    obj.insert("fileId", QJsonValue(fileId));


    return obj;
}

QPair<int, QString> Serialize::openCloseFileUnserialize(QJsonObject obj)
{
    /*
    Questa funzione de-serializza i nome del file
    INPUT:
    - obj: e' un Qjson che contiene tutte le info
    RETURN:
    - una QstringList con il nome del file
    */
    QPair<int, QString> user_file;
    user_file.first = obj.value("fileId").toInt();
    user_file.second = obj.value("user").toString();

    return user_file;
}

QJsonObject Serialize::newFileSerialize(QString filename, QString username, int type){
    /*
    Questa funzione serializza lil nome del file quando vuole fare una NEW, cio e' discriminato dal valore di type
    INPUT:
    - filename: stringa che contiene il nome del file da creare
    - username: stringa che rappresenta il client che ha fatto richiesta di creazione del file
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, i tipi che possono esere passati qui sono OPEN O CLOSE
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson
    */

    QJsonObject obj;
    obj.insert("type", QJsonValue(type));
    obj.insert("user", username);
    obj.insert("filename", QJsonValue(filename));
    return obj;
}
QPair<QString, QString> Serialize::newFileUnserialize(QJsonObject obj){

    QPair<QString, QString> user_file;
    user_file.first = obj.value("filename").toString();
    user_file.second = obj.value("user").toString();

    return user_file;
}




QJsonObject Serialize::messageSerialize(int fileId, Message message, int type)
{
    /*
    Questa funzione serializza i messaggi che vengono generati--> al suo interno sono contenuti il symbolo (lettera) e cosa si deve fare
    ossia insert/delte/style etc--> queste informazioni non sono contunte nel file json ma nel messaggio stesso
    INPUT:
    - message: messaggio che contiene il symbolo, cosa fare e tutte le informazioni necessarie
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, qui è accettato solo il tipo MESSAGE che indica sul client di inviare un messaggio
     e sul server di inoltrarlo ad altri client connessi ed insieme aggiornareil crdt locale sul server
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson e terminata con \r\n-> vedere se serve effettivamente altrimenti eliminare
    */
    QJsonObject obj;

    obj.insert("fileid", fileId);
    obj.insert("type", QJsonValue(type));
    int action = message.getAction();//insert, delete ecc.

    int senderId = message.getSenderId();//id del client o di chi manda

    obj.insert("action", QJsonValue(action));
    obj.insert("sender", QJsonValue(senderId));

    /*-------------------------------------------------------------------------------------------------------------------------------
    Nuovo elemento--> messagio che contine la posizione del cursore, se ciò accade il simbolo all'interno sar�  vuoto e la posizione diversa da zero
    controlliamo quindi prima questo caso particolare in modo da non eseguire il codice seguente piu lungo
    ----------------------------------------------------------------------------------------------------------------------------------*/
    if (message.getCursorPosition() > 0) {
        obj.insert("cursor_position", QJsonValue(message.getCursorPosition()));
        return obj;
    }

    Symbol s = message.getSymbol();

    char c = s.getChar();

    obj.insert("character", QJsonValue(c));

    //include di vector e array sono gia in symbol includso in message
    std::array<int, 2> id = s.getId();

    QJsonArray  Qid = { id[0],id[1] };//id globale

    obj.insert("globalCharacterId", QJsonValue(Qid));

    std::vector<int> v = s.getPos();

    QJsonArray Qvett;

    for (int i : v) {

        Qvett.append(QJsonValue(i));
    }

    obj.insert("position", QJsonValue(Qvett));


    //font e colore
    QFont font = s.getFont();
    QString serialFont = font.toString();
    obj.insert("font", QJsonValue(serialFont));

    QString color = s.getColor().name();//hex value
    Qt::AlignmentFlag aligment = s.getAlignment();

    //int red = color.red();
    //int green = color.green();
    //int blue = color.blue();

    //obj.insert("red",QJsonValue(red));
    //obj.insert("green", QJsonValue(green));
    //obj.insert("blue", QJsonValue(blue));
    obj.insert("color", color);
    obj.insert("alignment", aligment);


    //QJsonDocument doc(obj);
    //QString strJson(doc.toJson(QJsonDocument::Compact));
    //return strJson.append("\r\n");

    return obj;
}

QPair<int, Message> Serialize::messageUnserialize(QJsonObject obj)
{
    /*
    Questa funzione de-serializza i messaggi
    INPUT:
    - obj: è un Qjson che contiene tutte le info
    RETURN:
    - un oggetto di tipo message che può essere usato chiamando la funzione process del crdt per aggiornare sia su client/server
    vedi Message.h/cpp per specifiche
    */

    int fileid = obj.value("fileid").toInt();

    int action = obj.value("action").toInt();
    int sender = obj.value("sender").toInt();

   // QPair<int, Message> ret;
    //ret.first = fileid;


    /*-------------------------------------------------------------------------------------------------------------------------------
    Nuovo elemento--> messagio che contine la posizione del cursore, se ciò accade il simbolo all'interno sar�  vuoto e la posizione diversa da zero
    controlliamo quindi prima questo caso particolare in modo da non eseguire il codice seguente piu lungo
    ----------------------------------------------------------------------------------------------------------------------------------*/
    if (action == CURSOR) {
        __int64 cursorPosition = obj.value("cursor_position").toInt();
        Message m(cursorPosition, action, sender);
        QPair<int, Message> ret(fileid, m);


        //ret.second.operator=(m);
        return  ret;
    }

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


    int align = obj.value("alignment").toInt();
    Qt::AlignmentFlag alignFlag = static_cast<Qt::AlignmentFlag>(align);


    Symbol s(c, a, pos, font, color, alignFlag);


    Message m(s, action, sender);

    QPair<int, Message> ret(fileid, m);

    return ret;
}

QJsonObject Serialize::textMessageSerialize(QString str, int type)//non mi ricordo a che serviva--> forse per messaggi testuali dal server
{

    QJsonObject obj;

    obj.insert("type", QJsonValue(type));//??
    obj.insert("message", QJsonValue(str));


    //QJsonDocument doc(obj);
    //QString strJson(doc.toJson(QJsonDocument::Compact));
    //return strJson.append("\r\n");

    return obj;
}

QString Serialize::textMessageUnserialize(QJsonObject obj)
{

    return obj.value("message").toString();
}

//internal usage only----------------------------------------------------------

QJsonValue  Serialize::jsonValFromPixmap(const QPixmap& p) {
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    p.save(&buffer, "PNG");
    auto const encoded = buffer.data().toBase64();
    return { QLatin1String(encoded) };
}

QPixmap  Serialize::pixmapFrom(const QJsonValue& val) {
    auto const encoded = val.toString().toLatin1();
    QPixmap p;
    p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
    return p;
}
///---------------------------------------------------------------------------------------

QJsonObject Serialize::imageSerialize(QPixmap img, int type)//DA FINIRE NON SO COME VOGLIAMO GESTIRE LE IMMAGINI
{
    QJsonObject obj;

    obj.insert("type", QJsonValue(type));//??

    obj.insert("img", Serialize::jsonValFromPixmap(img));


    //QJsonDocument doc(obj);
    //QString strJson(doc.toJson(QJsonDocument::Compact));
    //return strJson.append("\r\n");

    return obj;
}

QPixmap Serialize::imageUnserialize(QJsonObject obj)
{
    QPixmap map;

    map = Serialize::pixmapFrom(obj.value("img"));

    return map;
}
//-------------------------------------------------------------------------------



QJsonObject Serialize::responseSerialize(bool res, QString message, int type, int userID, QColor color)
{
    /*
  res: da fare insieme a chi fa il server dato che sono i messaggi di rispost tipo ok/denied ecc codificati come intero
  Type: qui dovrebbe essere sempre SERVER_ANSWER
  */

  /*
  bool: successo o fallimento (OK, ERROR)
  message: risposta del server
  Type: qui dovrebbe essere sempre SERVER_ANSWER
  */
    QJsonObject obj;

    obj.insert("type", QJsonValue(type));//??

    obj.insert("res", QJsonValue(res));


    obj.insert("message", QJsonValue(message));

    obj.insert("userID", QJsonValue(userID));

    return obj;
}

QStringList Serialize::responseUnserialize(QJsonObject obj)
{
    /*
    Questa funzione de-serializza la risposta del server
    INPUT:
    - obj: e' un Qjson che contiene tutte le info sulla risposta data dal server
    RETURN:
    - una QstringList che di lunghezza 2:
    list[0]: valore booleano che mi dice OK/ERROR
    list[1]: stringa eventuale mandata dal server per messaggi piu complessi
    list[2]: userID
    */
    QStringList list;
    bool res = obj.value("res").toBool();
    list.append(res ? "true" : "false");
    list.append(obj.value("message").toString());
    list.append(obj.value("userID").toString());

    return list;
}

QJsonObject Serialize::ObjectFromString(QString& in)
{
    //Ritorna un Qjson a partire dalla stringa ricevuta ed elimina il \n\r
    QJsonObject obj;
    in = in.remove(in.length() - 5, 4); // elimino le ultime 4 lettere

    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if (!doc.isNull())
    {
        if (doc.isObject())
        {
            obj = doc.object();
        }
        else
        {
            std::cout << "Document is not an object" << std::endl;
        }
    }
    else
    {
        std::cout << "Invalid JSON...\n" << in.toStdString() << std::endl;
    }

    return obj;
}

QJsonObject Serialize::cursorPostionSerialize(int position, int userID, int type)
{
    /*
    Questa funzione serializza la posizione del cursore da mandare ai vari client
    INPUT:
    - position: posizione relativa del cursore all'interno del documento
    - userID : ID univoco legato ad un det client che midice a quale account mi sto riferendo
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, qui e' accettato solo il tipo CURSOR
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson e terminata con \r\n-> vedere se serve effettivamente altrimenti eliminare
*/
    QJsonObject obj;

    obj.insert("position", position);
    obj.insert("userID", userID);
    obj.insert("type", QJsonValue(type));



    //QJsonDocument doc(obj);
    //QString strJson(doc.toJson(QJsonDocument::Compact));
    //return strJson.append("\r\n");
    return obj;
}

std::vector<int> Serialize::cursorPostionUnserialize(QJsonObject obj)
{
    /*
    Questa funzione de-serializza la posizione del cursore e l'utente interessato
    INPUT:
    - obj: e' un Qjson che contiene tutte le info dell'utente come username password e nickname per fare login o signup
    RETURN:
    - un vector di lunghezza 2.
        list[0]: posizione relativa del cursore nel documento
        list[1]: id dell'utnte a cui e' riferit il cursore
    */
    std::vector<int> vett;
    vett.push_back(obj.value("position").toInt());
    vett.push_back(obj.value("userID").toInt());

    return vett;

}

QByteArray Serialize::fromObjectToArray(QJsonObject obj)
{
    QByteArray qarray = QJsonDocument(obj).toJson();
    return qarray;
}

QJsonObject Serialize::fromArrayToObject(QByteArray data)
{
    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject obj = document.object();
    return obj;
}


//QJsonObject Serialize::unserialize(QString str)// OLD
//{
//	QJsonDocument doc = QJsonDocument::fromJson(str.toLatin1());
//
//	QJsonObject obj = doc.object();
//
//	return obj;
//}

int Serialize::actionType(QJsonObject obj)
{
    //Ritorna l'intero che rappresenta il tipo di azione a partire da un Json
    return obj.value("type").toInt();
}

//void Serialize::setType(QString type)
//{
//	this->type = type;
//}
