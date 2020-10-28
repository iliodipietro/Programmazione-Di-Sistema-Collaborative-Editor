#include "Serialize.h"
#include "CRDT/CRDT.h"
#include <QBuffer>
#include <QJsonDocument>
#include <QDebug>


Serialize::Serialize(QWidget* parent)
{
}



QJsonObject Serialize::userSerialize(QString username, QString email, QString password, QString profileImage, int type)
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
    obj.insert("user", QJsonValue(username));
    obj.insert("password", QJsonValue(password));

    if (type == REGISTER) {
        //l'email serve solo in fase di register per salvarlo sul server
        obj.insert("email", QJsonValue(email));
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
    - una QstringList che puo avere lunghezza 2 0 5.
    -->lunghezza 2 se LOGIN:
        list[0]: username
        list[1]: password
    -->lunghezza 3 se REGISTER:
        list[0]: username
        list[1]: password
        list[2]: email
        list[3]: img
    */

    QString usr = obj.value("user").toString();
    QString password = obj.value("password").toString();

    //QString type = obj.value("type").toString();

    QStringList list;
    list.append(usr);
    list.append(password);

    if (Serialize::actionType(obj) == REGISTER) {
        QString email = obj.value("email").toString();
        QString img = obj.value("img").toString();

        list.append(email);
        list.append(img);
        //qDebug() << img;
    }

    return list;
}



QJsonObject Serialize::FileListSerialize(QMap<int, QString> files, int type){

    /*
    Questa funzione, una volta che tutti i file di un client sono stati correttamente serializzati nell'array files, lega l'utente ai file.
    INPUT:
    - username: stringa che contiene il nome dell'utente;
    - files: array serializzato contenente i campi(filename e id) per ogni file posseduto dal singolo client;
    - type: intero che è definito in define.h come SEND_FILES
    RETURN:
    - files: l'array che viene man mano aggiornato ad ogni chiamata.
    */

    QJsonObject obj;
    QList<int> ids = files.keys();
    QList<QString> names = files.values();
    int i;

    //obj.insert("fileid", QJsonValue(ids));
    //obj.insert("filename", QJsonValue(names));


    for(i = 0; i < files.size(); i++){
        QString fileid = "fileid" + QString(i);
        QString filename = "filename" + QString(i);
        obj.insert(fileid, ids[i]);
        obj.insert(filename, names[i]);
    }
    obj.insert("dim", files.size());
    obj.insert("type", type);

    return obj;
}

QMap<int, QString> Serialize::fileListUnserialize(QJsonObject obj){

    /*
    Questa funzione de-serializza il nome utente e tutti i file che possiede
    INPUT:
    - obj: e' un Qjson che contiene tutte le info
    RETURN:
    - una QPair<QString, QMap<int, QString>> con il nome dell'utente come primo elemento del QPair e come secondo una mappa contenente (fileId e FileName) di tutti i file dell'utente
    */

    QMap<int, QString> fileList;
    int size = obj.value("dim").toInt();
    int i;

    for(i = 0; i < size; i++){
        QString fileid = "fileid" + QString(i);
        QString filename = "filename" + QString(i);
        fileList.insert(obj.value(fileid).toInt(), obj.value(filename).toString());
    }

    return fileList;
}

QJsonObject Serialize::closeFileSerialize(int fileId, int siteCounter, int type){
    QJsonObject obj;

    obj.insert("fileId", fileId);
    obj.insert("sitecounter", siteCounter);
    obj.insert("type", type);

    return obj;
}

QPair<int, int> Serialize::closeFileUnserialize(QJsonObject obj){
    QPair<int, int> res;
    res.first = obj.value("fileId").toInt();
    res.second = obj.value("sitecounter").toInt();
    return res;

}


QJsonObject Serialize::openDeleteFileSerialize(int fileId, int type)
{
    /*
    Questa funzione serializza l'id del file quando vuole fare un OPEN o CLOSE o DELETE, cio e' discriminato dal valore di type
    INPUT:
    - fileId: intero che contiene l'id del file
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, i tipi che possono esere passati qui sono OPEN O CLOSE O DELETE
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson
    */
    QJsonObject obj;
    obj.insert("type", QJsonValue(type));
    obj.insert("fileId", QJsonValue(fileId));


    return obj;
}

int Serialize::openDeleteFileUnserialize(QJsonObject obj)
{
    /*
    Questa funzione de-serializza i nome del file
    INPUT:
    - obj: e' un Qjson che contiene tutte le info
    RETURN:
    - una QstringList con il nome del file
    */
    int fileId;
    fileId = obj.value("fileId").toInt();

    return fileId;
}

QJsonObject Serialize::newFileSerialize(QString filename,int id, int type){
    /*
    Questa funzione serializza lil nome del file quando vuole fare una NEW, cio e' discriminato dal valore di type
    INPUT:
    - filename: stringa che contiene il nome del file da creare
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, il tipo che pu� esere passato qui �
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson
    */

    QJsonObject obj;
    obj.insert("type", QJsonValue(type));
    obj.insert("fileId", QJsonValue(id));
    obj.insert("filename", QJsonValue(filename));
    return obj;
}
QString Serialize::newFileUnserialize(QJsonObject obj){

    QString filename = obj.value("filename").toString();

    return filename;
}

QJsonObject Serialize::renameFileSerialize(QString oldName, QString newName, int type){
    /*
    Questa funzione serializza l'Id del file, l'utente che vuole rinominarlo e il nuovo nome, in caso di una RENAME( cio e' discriminato dal valore di type)
    INPUT:
    - fileId: stringa che contiene l'id del file da rinominare
    -newName: nuovo nome da dare al file
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, il tipo che pu� esere passato qui � RENAME
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson
    */
    QJsonObject obj;
    obj.insert("oldname", oldName);
    obj.insert("newname",newName);
    obj.insert("type", type);
    return obj;

}

QPair<int, QString> Serialize::renameFileUnserialize(QJsonObject obj){
    QStringList list;
    QPair<int, QString> res;
    res.first = obj.value("fileId").toInt();
    res.second = obj.value("newname").toString();


    return res;
}


QJsonObject Serialize::sharedFileAcquisitionSerialize(QString URI, int type){
    /*
    Questa funzione serializza l'URI del file e l'utente che l'ha ricevuto e lo sta provando ad aprire, in caso di una SHARE( cio e' discriminato dal valore di type)
    INPUT:
    - URI: stringa che contiene l'URI del file da condividere
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, il tipo che pu� esere passato qui � SHARE
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson
    */
    QJsonObject obj;
    obj.insert("type", QJsonValue(type));
    obj.insert("URI", URI);
    return obj;
}

QString Serialize::sharedFileAcquisitionUnserialize(QJsonObject obj){

    return obj.value("URI").toString();
}

QJsonObject Serialize::changePasswordSerialize(QString oldPassword, QString newPassword, int type){
    /*
    Questa funzione serializza la vecchia e nuova password dell'utente che vuole cambiare password, in caso di una CHANGE_PASSWORD( cio e' discriminato dal valore di type)
    INPUT:
    - oldPassword: stringa che contiene la vecchia password
    - newPassword: nuova password
    - type: intero che basandomi sul file define.h mi dice cosa devo fare, il tipo che pu� esere passato qui � SHARE
    RETURN:
    - una Qstring che contiene il tutto serializzano come QJson
    */
    QJsonObject obj;
    obj.insert("oldpassword", oldPassword);
    obj.insert("newpassword", newPassword);
    obj.insert("type", QJsonValue(type));

    return obj;
}

QStringList Serialize::changePasswordUnserialize(QJsonObject obj){
    QStringList list;

    list.append(obj.value("oldpassword").toString());
    list.append(obj.value("newpassword").toString());

    return list;
}

QJsonObject Serialize::changeProfileSerialize(QString oldUsername, QString newUsername, QString oldEmail, QString newEmail, QString newImage, int type){

    QJsonObject obj;
    obj.insert("oldusername", oldUsername);
    obj.insert("newusername", newUsername);

    obj.insert("oldemail", oldEmail);
    obj.insert("newemail", newEmail);

    obj.insert("newimage", newImage);
    obj.insert("type", QJsonValue(type));
    return obj;

}

QStringList Serialize::changeProfileUnserialize(QJsonObject obj){
    QStringList list;

    list.append(obj.value("oldusername").toString());
    list.append(obj.value("newusername").toString());

    list.append(obj.value("oldemail").toString());
    list.append(obj.value("newemail").toString());
    list.append(obj.value("newimage").toString());

    return list;

}

QJsonObject Serialize::URISerialize(QString URI, int type){
    QJsonObject obj;

    obj.insert("URI", URI);
    obj.insert("type", type);

    return obj;

}

QJsonObject Serialize::changeProfileResponseSerialize(bool res, QString username, QString email, QString image, QString message, int type){
    QJsonObject obj;

    obj.insert("res", res);
    obj.insert("username", username);
    obj.insert("email", email);
    obj.insert("image", image);
    obj.insert("message", message);
    obj.insert("type", type);

    return obj;
}

QJsonObject Serialize::siteCounterSerialize(int fileId, int siteCounter, int type){
    QJsonObject obj;

    obj.insert("fileId", fileId);
    obj.insert("siteCounter", siteCounter);
    obj.insert("type", type);

    return obj;

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

    obj.insert("fileId", fileId);
    obj.insert("type", QJsonValue(type));
    int action = message.getAction();//insert, delete ecc.

    int senderId = message.getSenderId();//id del client o di chi manda

    obj.insert("action", QJsonValue(action));
    obj.insert("sender", QJsonValue(senderId));

    /*-------------------------------------------------------------------------------------------------------------------------------
    Nuovo elemento--> messagio che contine la posizione del cursore, se ciò accade il simbolo all'interno sar�  vuoto e la posizione diversa da zero
    controlliamo quindi prima questo caso particolare in modo da non eseguire il codice seguente piu lungo
    ----------------------------------------------------------------------------------------------------------------------------------*/
    if (message.getCursorPosition().size() > 0) {
            QJsonArray cursorPos;

            for (int i : message.getCursorPosition()) {

                cursorPos.append(QJsonValue(i));
            }

            obj.insert("cursor_position", QJsonValue(cursorPos));
            obj.insert("isSelection", QJsonValue(message.getIsSelection()));
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

    int fileid = obj.value("fileId").toInt();

    int action = obj.value("action").toInt();
    int sender = obj.value("sender").toInt();

   // QPair<int, Message> ret;
    //ret.first = fileid;


    /*-------------------------------------------------------------------------------------------------------------------------------
    Nuovo elemento--> messagio che contine la posizione del cursore, se ciò accade il simbolo all'interno sar�  vuoto e la posizione diversa da zero
    controlliamo quindi prima questo caso particolare in modo da non eseguire il codice seguente piu lungo
    ----------------------------------------------------------------------------------------------------------------------------------*/
    if (action == CURSOR_S) {
            std::vector<int> cursorPosition;

            QJsonArray vettCursorPos = obj.value("cursor_position").toArray();

            for (QJsonValue qj : vettCursorPos) {

                cursorPosition.push_back(qj.toInt());
            }

            bool isSelection = obj.value("isSelection").toBool();
            Message m(cursorPosition, action, sender, isSelection);
            return QPair<int, Message>(fileid, m);
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

    if (action == DELETE_SYMBOL) {
        Symbol s(c, a, pos, QFont(), QColor(), Qt::AlignmentFlag());
        Message m(s, action, sender);
        return QPair<int, Message>(fileid, m);
    }

    QFont font;
    font.fromString(obj.value("font").toString());
    //QFont font(obj.value("font").toString());
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



QJsonObject Serialize::responseSerialize(bool res, QString message, int type, QString username, QString email, int userID, QColor userColor)
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

    QString color = userColor.name();
    obj.insert("color", color);

    obj.insert("email", email);
    obj.insert("username", username);

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
    list[3]: userColor
    list[4]: email
    list[5]:username
    */
    QStringList list;
    bool res = obj.value("res").toBool();
    list.append(res ? "true" : "false");
    list.append(obj.value("message").toString());
    list.append(obj.value("userID").toString());
    list.append(obj.value("color").toString());
    list.append(obj.value("email").toString());
    list.append(obj.value("username").toString());

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

QJsonObject Serialize::addEditingUserSerialize(int userId, QString username, QColor userColor, int fileId, int type){
    /*Funzione usata per inviare a tutti i client lo userId e lo username del client che si ha appena aperto un file
      Viene anche usata per mandare al client che ha appena aperto il file le informazioni riguardanti tutti i client gi� connessi al file
    */

    QJsonObject obj;
    obj.insert("userId", userId);
    obj.insert("username", username);
    QString color = userColor.name();
    obj.insert("userColor", color);
    obj.insert("fileId", fileId);
    obj.insert("type", QJsonValue(type));

    return obj;
}

QPair<int, QStringList> Serialize::addEditingUserUnserialize(QJsonObject obj) {
    /*Funzione usata per de-serializzare le informazioni riguardanti al client che ha aperto il file
      OUTPUT:
      un QPair<int,QStringList> contenente:
        - l'id del file a cui � relativo il messaggio
        una lista di lunghezza 3 contenentee:
            -list[0] -> userId
            -list[1] -> username
            -list[2] -> userColor
    */

    QStringList sl;
    sl.append(obj.value("userId").toString());
    sl.append(obj.value("username").toString());
    sl.append(obj.value("userColor").toString());

    return QPair<int, QStringList>(obj.value("fileId").toInt(), sl);
}

QJsonObject Serialize::removeEditingUserSerialize(int userId, int fileId, int type){
    /*Funzione usata per inviare a tutti i client lo userId del client che ha chiuso il file
    */

    QJsonObject obj;
    obj.insert("userId", userId);
    obj.insert("fileId", fileId);
    obj.insert("type", QJsonValue(type));

    return obj;
}

QPair<int, int> Serialize::removeEditingUserUnserialize(QJsonObject obj){
    /*Funzione usata per de-serializzare lo userid del client che ha chiuso il file
        OUTPUT:
        una coppia di interi contenente:
        -key -> lo userId
        -value -> il fileId
    */

    int userId = obj.value("userId").toInt();
    int fileId = obj.value("fileId").toInt();
    QPair<int, int> userFile(fileId, userId);

    return userFile;
}

QJsonObject Serialize::logoutUserSerialize(int type)
{
    //Funzione usata per creare un messaggio che contiene al suo interno solo la richiesta di logout dell'utente
    QJsonObject obj;
    obj.insert("type", QJsonValue(type));

    return obj;
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
