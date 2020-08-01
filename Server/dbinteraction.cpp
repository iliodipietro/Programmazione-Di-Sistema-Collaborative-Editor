#include "dbinteraction.h"


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
                qDebug("DB connection established\n");
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
        cnt = query.value(0).toInt();
        if(cnt > 0){
            qDebug("username already exists\n");
            //inviare messaggio di errore sul socket
            message = "SERVER_ERROR";
           // response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
            socket->write(response);
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
                message = "OK";
             //   response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, message, SERVER_ANSWER));
                socket->write(response);

            }
            else{
                qDebug("INSERT failed\n");
                //inviare messaggio di errore sul socket
                 message = "SERVER_ERROR";
               // response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                socket->write(response);
            }
        }
    }
    else {
        qDebug("SELECT COUNT query failed!\n");
        //inviare messaggio di errore sul socket
         message = "SERVER_ERROR";
       // response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        socket->write(response);
    }

    return;
}

void DBInteraction::login(QString username, QString password, QTcpSocket *socket){



    return ;


}
