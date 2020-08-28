#include "dbinteraction.h"



/*
    TO DO:
        - aggiunta e gestione delle immagini di profilo;
*/

DBInteraction *DBInteraction::instance = nullptr;

DBInteraction::DBInteraction(){


}

DBInteraction* DBInteraction::startDBConnection(){

    if(!instance){
        instance = new DBInteraction();
        const QString DRIVER("QSQLITE");
        if(QSqlDatabase::isDriverAvailable(DRIVER)){
            qDebug("driver avaiable!\n");

            instance->db = QSqlDatabase::addDatabase(DRIVER);
            instance->db.setDatabaseName("project.db");
            if(!instance->db.open()){

                qDebug("DB connection failed\n");
            }
            else {
                qDebug("DB connection established!!!\n");
            }
        }
        else {
            qDebug("error: drivers not avaiable\n");
        }

    }

    return DBInteraction::instance;
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

void DBInteraction::sendMessage(QTcpSocket *socket, QByteArray obj){
    // questa funzione invia per prima cosa la dimensione dell'oggetto serializzato sul socket, per poi inviare l'oggetto vero e proprio.

    if(socket->state() == QAbstractSocket::ConnectedState){
           qint32 msg_size = obj.size();
           QByteArray toSend;
           socket->write(toSend.number(msg_size), sizeof (quint64)); //la funzione number converte il numero che rappresenta la dimensione del dato da inviare (msg_size) in stringa (es. 100 --> "100").
                                                                     //Siccome una stringa occupa di piu del relativo numero ("100" occupa 8*3 bit mentre 100 ne occupa solo 8), tale stringa viene mandata sul socket
                                                                     // su 64 bit invece di 32 che rappresenta la massima dimensione possibile di un dato
           socket->waitForBytesWritten();
           qint32 byteWritten = 0;
           while(byteWritten < msg_size){
               byteWritten += socket->write(obj);
               socket->waitForBytesWritten();
           }
       }

}


void DBInteraction::registration(QString username, QString password, QTcpSocket *socket){

    QSqlQuery query;
    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QByteArray response;
    QString message;
    int cnt = 0;

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

            query.prepare("INSERT INTO users(username, password, salt), VALUES ((:username), (:hashed_pwd), (:salt))");
            query.bindValue(":username", username);
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

    return;
}

void DBInteraction::login(QString username, QString password, QTcpSocket *socket){

    QSqlQuery query;
    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QByteArray response;
    QString message;
    QJsonArray files;
    int cnt = 0;
    bool err = false;

    query.prepare("SELECT COUNT(*) FROM users WHERE Username = (:username)");
    query.bindValue(":username", username);
    if(query.exec()){

        if(query.next()){
            cnt = query.value(0).toInt();
        }
        if(cnt == 1){
            QSqlQuery query;
            qDebug()<<"checking password...\n";
            query.prepare("SELECT Password, Salt FROM users WHERE Username = (:username)");
            query.bindValue(":username", username);
            if (query.exec()) {

                if(query.next()){

                    salt = QString(query.value("Salt").toString());
                    salted_pwd = password.append(salt).toUtf8();
                    hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));

                    if(hashed_pwd.compare(QString(query.value("Password").toString()))){
                        //success
                        users.insert(username);
                        QSqlQuery query2;
                        query2.prepare("SELECT FileName, Id FROM files WHERE UserName =(:username)");
                        query2.bindValue(":username", username);

                        if(query.exec()){

                            while(query.next()){
                                //per ogni file creo un jsonObjest contenente nome del file, id del file e numero di client che lo stanno condividendo (?)

                                QString filename = query2.value("FileName").toString();
                                int fileId = query2.value("Id").toInt();

                                if(!users_files.contains(username)){//funzionerà!?!?!?!?!?!?
                                    users_files.insert(username, QMap<int, QString>{{fileId, filename}});
                                }
                                else{
                                    users_files[username].insert(fileId, filename);

                                }

                               // QSqlQuery query3;
                                files = Serialize::singleFileSerialize(filename, fileId, files);
                            }
                            response = Serialize::fromObjectToArray(Serialize::user_filesSerialize(username, files, LOGIN));
                            //sendMessage(socket, response);
                        }
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
        //sendMessage(socket, response);
    }

    sendMessage(socket, response);

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


    query.prepare("SELECT COUNT(*) FROM files WHERE UserName = (:username) AND FileName = (.filename)");
    query.bindValue(":username", username);
    query.bindValue(":filename", filename);
    if(query.exec()){
        if(query.next()){
            cnt = query.value(0).toInt();
        }
        if(cnt != 0){
            qDebug("ERROR: the file does already exist!\n");
            //fare altro???

        }
        else{
            //il file non esiste, quindi posso crearlo
            QSqlQuery query2;
            query2.prepare("SELECT COUNT(Id) FROM files"); //l'id del file è un intero crescente
            if(query2.exec()){
                if(query2.next()){
                    fileId = query2.value(0).toInt();
                }

                QSqlQuery query3;
                query3.prepare("INSERT INTO files(FileName, Id, userName) VALUES ((:filename), (fileId), (username))");
                query3.bindValue(":filename", filename);
                query3.bindValue(":fileId", fileId);
                query3.bindValue(":username", username);

                if(query3.exec()){
                    QFile file(QString::number(fileId));
                    if(file.open(QIODevice::ReadWrite)){
                        //che cazzo devo fareeeeeeee

                    }
                    else {
                        qDebug() << "file not opened\n";
                    }


                }
                else {
                    qDebug() << "INSERT failed: " << query2.lastError() << "\n";
                }


            }
            else {
                qDebug() << "SELECT COUNT(Id) failed: " << query2.lastError() << "\n";
            }

        }
    }
    else {
        qDebug()<< "SELECT COUNT(*) NOT executed: " << query.lastError() << "\n";
    }


}

void DBInteraction::openFile(int fileId, QString username, QTcpSocket *socket){

    if(users_files.contains(username)){
        if(users_files[username].contains(fileId)){
            //SUCCESS
            qDebug() <<"il client: "<< username << "HA accesso al file richiesto\n";

            if(files.contains(fileId)){
                //il file è gia in RAM
            }
            else {
                //cercare il file nel DB ????
            }



        }
        else{
            qDebug() <<"il client: "<< username << "NON ha accesso al file richiesto\n";

        }
    }
    else{
        qDebug() <<"il client: "<< username << "NON ha accesso al file richiesto\n";

    }


}


void DBInteraction::closeFile(int fileId, QString username, QTcpSocket *socket){


}






void DBInteraction::searchFile(){}
void DBInteraction::deleteFile(){}
