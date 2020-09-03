#include "dbinteraction.h"
#include <QDir>
#include <QDataStream>

/*
    TO DO:
        - aggiunta e gestione delle immagini di profilo;


*/

DBInteraction *DBInteraction::instance = nullptr;

DBInteraction::DBInteraction(){


}

DBInteraction* DBInteraction::startDBConnection(){
    bool err = false;
    QString path = QDir::currentPath().append("\\project.sqlite"); //project.db";scegliere path in cui salvare il DB

    if(!instance){
        instance = new DBInteraction();
        const QString DRIVER("QSQLITE");
        bool exist = false;


        if(QSqlDatabase::isDriverAvailable(DRIVER)){
            qDebug("driver avaiable!\n");

            if(QFile::exists(path)){
                exist = true;
            }

            instance->db = QSqlDatabase::addDatabase(DRIVER);
            instance->db.setDatabaseName(path);



            if(!instance->db.open()){ //la open apre il db se gia esistente oppure ne crea uno nuovo in caso non esista.
                                      //In quest'ultimo caso devo creare le tabelle che lo compongono, quindi prima verifico l'esistenza del file (riga sopra) e se non esiste, creo le tabelle

                qDebug("DB connection failed\n");
                err = true;
            }
            else {
                qDebug("DB connection established!!!\n");

                if(!exist){
                    qDebug("creation of tables!\n");
                    QSqlQuery query1, query2;
                    query1.prepare("CREATE TABLE users ("
                                  "Username VARCHAR(255) primary key, "
                                  "UserId   INT          NOT NULL,"
                                  "Password VARCHAR(255) NOT NULL,"
                                  "Salt     VARCHAR(255) );");

                    query2.prepare("CREATE TABLE files ("
                                  "FileName VARCHAR(255) NOT NULL, "
                                  "Id       INT          NOT NULL,"
                                  "Username VARCHAR(255) NOT NULL,"
                                  "Path     VARCHAR(255),"
                                  "PRIMARY KEY(FileName, Username) );");
                    if(query1.exec() && query2.exec()){
                        qDebug("tables created!!!\n");
                    }
                }
                else {
                    qDebug("tables already existing!\n");
                }
                instance->db.close();
            }
        }
        else {
            qDebug("error: drivers not avaiable\n");
            err = true;
        }

    }
    if(err){
        return nullptr;
    }
    else {
        return DBInteraction::instance;
    }

}
QString DBInteraction::generateRandomString(int len) {
   /* QString alphabet = { '0','1','2','3','4','5','6','7','8','9',
                              'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                              'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };*/

    QString alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    QString res = "";
    for (int i = 0; i < len; i++)
        res = res.append(alphanum[rand() % 62]);

    return res;
}

//sarebbe bene non creare funzioni separate per la scrittura su socket ma usare quella nella classe ClientManager
void DBInteraction::sendMessage(QTcpSocket *socket, QByteArray obj){
    // questa funzione invia per prima cosa la dimensione dell'oggetto serializzato sul socket, per poi inviare l'oggetto vero e proprio.

    if(socket->state() == QAbstractSocket::ConnectedState){
           qint64 msg_size = obj.size();
           qDebug()<<msg_size<<"\n";
           socket->write(instance->intToArray(obj.size()).append(obj)); //la funzione number converte il numero che rappresenta la dimensione del dato da inviare (msg_size) in stringa (es. 100 --> "100").
                                                                     //Siccome una stringa occupa di piu del relativo numero ("100" occupa 8*3 bit mentre 100 ne occupa solo 8), tale stringa viene mandata sul socket
                                                                     // su 64 bit invece di 32 che rappresenta la massima dimensione possibile di un dato
           socket->waitForBytesWritten();
       }

    qDebug()<<"response sent\n";

}


void DBInteraction::registration(QString username, QString password, QTcpSocket *socket){

    QSqlQuery query;
    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QByteArray response;
    QString message;
    int cnt = 0;
    int userid = 1;

    if(instance->db.open()){ //bisogna aprire la connessione al db prima altrimenti non funziona

        query.prepare("SELECT COUNT(*) FROM users WHERE Username = (:username)");
        query.bindValue(":username", username); // no matching member function for call to 'bindValue' --> risolto con #incliude <QVariant>

        if(query.exec()){
            if(query.next()){
                cnt = query.value(0).toInt();
            }
            if(cnt > 0){
                qDebug("username already exists\n");
                //inviare messaggio di errore sul socket
                message = "SERVER_ERROR";
                response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                //sendMessage(socket, response);
            }
            else {
                qDebug("insertion...\n");
                salt = DBInteraction::generateRandomString(password.size());
                salted_pwd = password.append(salt).toUtf8();
                hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));
                qDebug()<<"new password: "<< hashed_pwd<<"\n";

                QSqlQuery query2;
                query2.prepare("SELECT COUNT(UserId) FROM users"); //l'id dell'utente � un intero crescente
                if(query2.exec()){
                    if(query2.next()){
                        userid = query2.value(0).toInt();
                    }
                }
                qDebug()<<"userid: "<< userid<<"\n";

                query.prepare("INSERT INTO users(username, userid, password, salt) VALUES ((:username),(:userid), (:password), (:salt))");
                query.bindValue(":username", username);
                query.bindValue(":userid", userid);
                query.bindValue(":password", hashed_pwd);
                query.bindValue(":salt", salt);

                if(query.exec()){
                    //success
                    qDebug("INSERT completed successfully!!\n");
                    //inviare messaggio di successo sul socket
                    message = "New user added!\n";
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, message, SERVER_ANSWER));
                    //sendMessage(socket, response);

                }
                else{
                    qDebug("INSERT failed\n");
                    qDebug()<<query.lastError();
                    //inviare messaggio di errore sul socket
                     message = "SERVER_ERROR";
                     response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                     //sendMessage(socket, response);
                }
            }

        }
        else {
            qDebug()<< "SELECT COUNT query failed!"<<query.lastError()<<"\n";
            //inviare messaggio di errore sul socket
             message = "SERVER_ERROR";
             response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
             //sendMessage(socket, response);
        }

        sendMessage(socket, response);
        instance->db.close();
    }

    sendMessage(socket, response);

    return;
}

void DBInteraction::login(QString username, QString password, QTcpSocket *socket){

    QSqlQuery query;
    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QByteArray response, response_ok;
    QString message;
    QJsonArray files; // la lista � vuota?
    int cnt = 0;
    int userid;
    bool err = false;

    if(instance->db.open()){

        query.prepare("SELECT COUNT(*) FROM users WHERE Username = (:username)");
        query.bindValue(":username", username);
        if(query.exec()){

            if(query.next()){
                cnt = query.value(0).toInt();
            }
            if(cnt == 1){
                QSqlQuery query;
                qDebug()<<"checking password...\n";
                query.prepare("SELECT Password, UserId, Salt FROM users WHERE Username = (:username)");
                query.bindValue(":username", username);
                if (query.exec()) {

                    if(query.next()){

                        salt = QString(query.value("Salt").toString());
                        salted_pwd = password.append(salt).toUtf8();
                        hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));

                        if(hashed_pwd.compare(QString(query.value("Password").toString())) == 0){
                            //success
                            userid = query.value("UserId").toInt();

                            instance->users.insert(socket, userid);

                            /*QSqlQuery query2;
                            query2.prepare("SELECT FileName, Id FROM files WHERE UserName =(:username)");
                            query2.bindValue(":username", username);

                            if(query2.exec()){

                                if(query2.size() > 0){

                                    while(query2.next()){
                                        //per ogni file creo un jsonObjest contenente nome del file e id

                                        QString filename = query2.value("FileName").toString();
                                        int fileId = query2.value("Id").toInt();

                                        files = Serialize::singleFileSerialize(filename, fileId, files);
                                    }
                                }*/
                                message = "login OK";
                                response_ok = Serialize::fromObjectToArray(Serialize::responseSerialize(true, message, SERVER_ANSWER));

                                sendMessage(socket, response_ok);
                                //response = Serialize::fromObjectToArray(Serialize::user_filesSerialize(userid, username, files, LOGIN));
                            //}
                        }
                        else{
                            //insuccess
                            qDebug()<< "Password not valid!!\n";
                            err = true;
                        }


                    }
                    else {
                        qDebug()<< "query.next() in SELECT Password error\n ";
                        err = true;
                    }
                }
                else {
                    qDebug()<< "SELECT Password NOT executed: "<< query.lastError()<<"\n";
                    err = true;
                }
            }
            else {
                qDebug()<< "Username not valid\n";
                err = true;
            }
        }
        else{
            qDebug()<< "SELECT COUNT(*) NOT executed: "<< query.lastError()<<"\n";
            err = true;
        }


        if(err){
            message = "AUTHENTICATION_ERROR";
            response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        }

        //sendMessage(socket, response);

        instance->db.close();
    }
    return ;
}


void DBInteraction::createFile(QString filename, QString username, QTcpSocket *socket){
    /*
        TO DO
            - popolare la mappa files
    */

    QSqlQuery query;
    int cnt = 0;
    int fileId = 0;
    QString path = "C:/Users/Ilio/Desktop/Progetto_Malnati_git/";
    QString message;
    QByteArray response;
    bool err = false;

    if(instance->db.open()){

        query.prepare("SELECT COUNT(*) FROM files WHERE UserName = (:username) AND FileName = (.filename)");
        query.bindValue(":username", username);
        query.bindValue(":filename", filename);
        if(query.exec()){
            if(query.next()){
                cnt = query.value(0).toInt();
            }
            if(cnt != 0){
                message = "ERROR: the file does already exist!\n";
                err = true;
                qDebug("ERROR: the file does already exist!\n");

            }
            else{
                //il file non esiste, quindi posso crearlo
                QSqlQuery query2;
                query2.prepare("SELECT COUNT(Id) FROM files"); //l'id del file � un intero crescente
                if(query2.exec()){
                    if(query2.next()){
                        fileId = query2.value(0).toInt();
                    }
                    path.append(username).append("/").append(filename).append(".txt"); //  esempio --> C:/Users/Ilio/Desktop/Progetto_Malnati_git/ilio/prova.txt
                    qDebug()<< "Path: " << path << "\n";


                    QSqlQuery query3;
                    query3.prepare("INSERT INTO files(FileName, Id, userName, Path) VALUES ((:filename), (:fileId), (:username), (:path))");
                    query3.bindValue(":filename", filename);
                    query3.bindValue(":fileId", fileId);
                    query3.bindValue(":username", username);
                    query3.bindValue(":path", path);

                    if(query3.exec()){
                        File *newfile = new File(fileId, path);
                        message = "OK file created\n";
                        response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, message, SERVER_ANSWER));
                        sendMessage(socket, response);

                        files.insert(fileId, newfile);
                        newfile->addUser(socket);
                    }
                    else {
                        message = "ERROR\n";
                        err = true;
                        qDebug() << "INSERT failed: " << query2.lastError() << "\n";
                    }
                }
                else {
                    message = "ERROR\n";
                    err = true;
                    qDebug() << "SELECT COUNT(Id) failed: " << query2.lastError() << "\n";
                }
            }
        }
        else {
            message = "ERROR\n";
            err = true;
            qDebug()<< "SELECT COUNT(*) NOT executed: " << query.lastError() << "\n";
        }

        if(err){
            response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
            sendMessage(socket, response);
        }
        instance->db.close();
    }
    return;

}

void DBInteraction::openFile(int fileId, QString username, QTcpSocket *socket){

    File *f = nullptr;
    QString message;
    QByteArray response;
    bool err = false;

    if(files.contains(fileId)){
        //il file � gia in RAM
        f = files.value(fileId);
    }
    else {
        //cercare il file nel DB
    }



    if(f != nullptr){
        message = "OK\n";
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, message, SERVER_ANSWER));
        f->addUser(socket);
    }
    else {

        err = true;
        message = "ERROR\n";
    }

    if(err){
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
    }

    sendMessage(socket, response);



}


void DBInteraction::closeFile(int fileId, QString username, QTcpSocket *socket){


}




void DBInteraction::deleteFile(){}

File* DBInteraction::getFile(int fileid){
    return files.value(fileid);
}

QByteArray DBInteraction::intToArray(qint64 source) {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}
