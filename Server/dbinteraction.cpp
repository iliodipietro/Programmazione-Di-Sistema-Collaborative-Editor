#include "dbinteraction.h"
#include <QDir>
#include <QDataStream>
#include <QRandomGenerator>

DBInteraction* DBInteraction::instance = nullptr;

DBInteraction::DBInteraction() {}

DBInteraction* DBInteraction::startDBConnection() {
    bool err = false;
    QString path = QDir::currentPath().append("/project.sqlite");

    if (!instance) {
        instance = new DBInteraction();
        const QString DRIVER("QSQLITE");
        bool exist = false;

        if (QSqlDatabase::isDriverAvailable(DRIVER)) {
            qDebug("driver avaiable!\n");
            // qDebug()<<"current path: "<<path <<"\n";

            if (QFile::exists(path)) {
                exist = true;
            }

            instance->db = QSqlDatabase::addDatabase(DRIVER);
            instance->db.setDatabaseName(path);

            if (!instance->db.open()) { //la open apre il db se gia esistente oppure ne crea uno nuovo in caso non esista.
                                      //In quest'ultimo caso devo creare le tabelle che lo compongono, quindi prima verifico l'esistenza del file (riga sopra) e se non esiste, creo le tabelle

                qDebug("DB connection failed\n");
                err = true;
            }
            else {
                qDebug("DB connection established!!!\n");

                if (!exist) {
                    qDebug("creation of tables!\n");
                    QSqlQuery query1, query2;
                    query1.prepare("CREATE TABLE users ("
                        "Username VARCHAR(255) NOT NULL, "
                        "UserId   INTEGER      PRIMARY KEY AUTOINCREMENT,"
                        "Password VARCHAR(255) NOT NULL,"
                        "Email    VARCHAR(255) NOT NULL,"
                        "Salt     VARCHAR(255),"
                        "ProfileImage VARCHAR(255) NOT NULL);");

                    query2.prepare("CREATE TABLE files ("
                        "FileId      INTEGER,"
                        "FileName    VARCHAR(255) NOT NULL, "
                        "UserId      INT          NOT NULL,"
                        "Path        VARCHAR(255),"
                        "SiteCounter INT,"
                        "PRIMARY KEY  (FileId, UserId));");

                    if (query1.exec() && query2.exec()) {
                        qDebug("tables created!!!\n");
                    }
                    else {
                        qDebug() << query2.lastError() << "\n";
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
    if (err) {
        instance->db.close();
        return nullptr;
    }
    else {
        instance->db.close();
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
/*void DBInteraction::sendMessage(QTcpSocket *socket, QByteArray obj){
    // questa funzione invia per prima cosa la dimensione dell'oggetto serializzato sul socket, per poi inviare l'oggetto vero e proprio.

    if(socket->state() == QAbstractSocket::ConnectedState){
           qint64 msg_size = obj.size();
           qDebug()<<msg_size<<"\n";
           socket->write(instance->intToArray(obj.size()).append(obj));
       }

    qDebug()<<"response sent\n";

}*/

void DBInteraction::sendError(ClientManager* client) {
    QString message;
    QByteArray response;

    message = "SERVER ERROR";
    response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
    client->writeData(response);
    //   sendMessage(socket, response);
    return;
}

void DBInteraction::sendSuccess(ClientManager* client) {
    QString message;
    QByteArray response;

    message = "OK";
    response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, message, SERVER_ANSWER));
    client->writeData(response);
    //sendMessage(socket, response);
    return;
}

bool DBInteraction::is_email_valid(QString email) {

    QRegularExpression re("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);   //("([a-z]+)([_.a-z0-9]*)([a-z0-9]+)(@)([a-z]+)([.a-z]+)([a-z]+)");      //("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
    QSqlQuery query;
    int cnt = 0;

    if (re.match(email).hasMatch()) {
        //controllo unicità della email
        if (instance->db.open()) {
            query.prepare("SELECT COUNT(*) FROM users WHERE email = (:email)");
            query.bindValue(":email", email);

            if (query.exec()) {
                if (query.next()) {
                    cnt = query.value(0).toInt();
                    if (cnt > 0) {
                        qDebug("email already exists\n");
                        instance->db.close();
                        return false;
                    }
                }
                instance->db.close();
                return true;
            }
            else {
                qDebug() << "SELECT COUNT2 query failed!" << query.lastError() << "\n";
                instance->db.close();
                return false;
            }

        }
        else {
            return false;
        }
  
    }
    else {
        return false;
    }

}

bool DBInteraction::is_username_unique(QString username){
    QSqlQuery query;
    int cnt = 0;

    if (instance->db.open()) {
        query.prepare("SELECT COUNT(*) FROM users WHERE Username = (:username)");
        query.bindValue(":username", username); // no matching member function for call to 'bindValue' --> risolto con #incliude <QVariant>

        if (query.exec()) {
            if (query.next()) {
                cnt = query.value(0).toInt();
                if (cnt > 0) {
                    instance->db.close();
                    return false;
                }
            }
        }
        else {
            qDebug() << "SELECT COUNT query failed!" << query.lastError() << "\n";
            instance->db.close();
            return false;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        return false;
    }
    return true;
}

QString DBInteraction::computeHashPassword(QString password) {
    QString hashed_pwd;
    QString salt;
    QByteArray salted_pwd;

    salt = instance->generateRandomString(password.size());
    salted_pwd = password.append(salt).toUtf8();
    hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));
    return hashed_pwd;
}

bool DBInteraction::checkPassword(QString password, ClientManager* client) {

    if (instance->db.open()) {
        QSqlQuery query;
        QByteArray salted_pwd;
        QString hashed_pwd;
        QString salt;
        QString message;
        QByteArray response;
        int userid = client->getId();
        qDebug() << "userid: " << userid << "\n";

        qDebug() << "checking password...\n";

        query.prepare("SELECT Password, Salt FROM users WHERE userid = (:userid)");
        query.bindValue(":userid", userid);
        if (query.exec()) {

            if (query.next()) {

                salt = QString(query.value("Salt").toString());
                salted_pwd = password.append(salt).toUtf8();
                hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));

                if (hashed_pwd.compare(QString(query.value("Password").toString())) == 0) {
                    qDebug() << "Password ok\n";
                    instance->db.close();
                    return true;
                }
                else {
                    qDebug() << "Password not valid!!\n";
                    message = "WRONG USERNAME OR PASSWORD";
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                    client->writeData(response);
                    // sendMessage(socket, response);
                    instance->db.close();
                    return false;
                }
            }
            else {
                qDebug() << "error\n";
                sendError(client);
                instance->db.close();
                return false;
            }
        }
        else {
            qDebug() << "SELECT Password failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return false;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return false;
    }
}

void DBInteraction::registration(QString username, QString email, QString password, QString profileImage, ClientManager* incomingClient) {

    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QByteArray response;
    QString message;
    int userid;

    /*CONTROLLO VALIDITà ED UNICITà DELLA EMAIL */
    if (!is_email_valid(email)) {
        message = "Email format is incorrect or the email used is not unique.";
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        incomingClient->writeData(response);
        return;

    }

    /*CONTROLLO UNICITÃ USERNAME  */
    if (!instance->is_username_unique(username)) {
        qDebug("username already exists\n");
        message = "Username already exist";
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        incomingClient->writeData(response);
        return;
    }

    if (instance->db.open()) { 

        /*INSERIMENTO DATI NEL DB*/
        qDebug("insertion...\n");

        QSqlQuery query;
        salt = instance->generateRandomString(password.size());
        salted_pwd = password.append(salt).toUtf8();
        hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));
        qDebug() << "new password: " << hashed_pwd << "\n";

        /*
        QSqlQuery query2;
        query2.prepare("SELECT COUNT(UserId) FROM users"); //l'id dell'utente Ã¨ un intero crescente
        if(query2.exec()){
            if(query2.next()){
                userid = query2.value(0).toInt() +1;
                qDebug()<<"userid: "<< userid<<"\n";
            }
        }
        else{
            qDebug()<< "SELECT COUNT(UserId) query failed!"<<query.lastError()<<"\n";
            sendError(incomingClient);
            instance->db.close();
            return;
        }*/

        QString images_directory_path(QDir::currentPath() + "\\ImmaginiProfilo\\");
        if (!QDir(images_directory_path).exists()) {
            //creo la cartella ImmaginiProfilo
            QDir().mkdir(images_directory_path);
        }

        QString path(QDir::currentPath() + "/ImmaginiProfilo/" + username + "_profileImage.txt");
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << profileImage;
            file.close();
        }
        else {
            qDebug("image not saved!!\n");
            message = "image not saved!\n";
            response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
            incomingClient->writeData(response);
            instance->db.close();
            return;
        }

        QString user_directory_path(QDir::currentPath() + "/files/" + "/" + username + "/");
        qDebug() << "cartella utente: " << user_directory_path << "\n";

        if (!QDir(user_directory_path).exists()) {
            //creo la cartella relativa all'utente
            QDir().mkdir(user_directory_path);
        }

        query.prepare("INSERT INTO users(username, userid, password, email, salt, profileImage) VALUES ((:username), NULL, (:password), (:email), (:salt), (:profileImage))");
        query.bindValue(":username", username);
        //query3.bindValue(":userid", userid);
        query.bindValue(":password", hashed_pwd);
        query.bindValue(":email", email);
        query.bindValue(":salt", salt);
        query.bindValue(":profileImage", path);

        if (query.exec()) {
            //success
            qDebug("new user added!\n");
            sendSuccess(incomingClient);
            instance->db.close();
        }
        else {
            qDebug("INSERT failed\n");
            qDebug() << query.lastError();
            sendError(incomingClient);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(incomingClient);
        return;
    }
    return;
}

void DBInteraction::login(QString username, QString password, ClientManager* incomingClient) {

    QSqlQuery query;
    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QString profileImage, profileImagePath;
    QByteArray response, response_ok;
    QString message;
    int cnt = 0;
    int userid;

    if (instance->db.open()) {

        query.prepare("SELECT COUNT(*),* FROM users WHERE Username = (:username)");
        query.bindValue(":username", username);
        if (query.exec()) {

            if (query.next()) {
                cnt = query.value(0).toInt();
            }
            if (cnt == 1) {
                userid = query.value("UserId").toInt();
                qDebug() << "userid nella login: " << userid << "\n";
                incomingClient->setUsername(username);
                incomingClient->setId(userid);

                if (checkPassword(password, incomingClient)) {
                    //success
                    profileImagePath = query.value("ProfileImage").toString();
                    QFile file(profileImagePath);

                    if (file.open(QIODevice::ReadOnly)) {
                        QTextStream stream(&file);
                        profileImage.append(stream.readAll());
                        file.close();
                    }
                    else {
                        qDebug("image not available!!\n");
                        message = "image not available!\n";
                        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                        incomingClient->writeData(response);
                        instance->db.close();
                        return;
                    }

                    QColor userColor = instance->generateRandomColor(userid);
                    QString email = query.value("email").toString();
                    incomingClient->setColor(userColor);
                    instance->activeusers.push_back(incomingClient);
                    // instance->users.insert(username, new ClientManager(userid,socket));
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, profileImage, SERVER_ANSWER, username, email, userid, userColor)); // ho dovuto modificare la funzione per includere anche l'username per poterla usare nel changeProfile. Per non cambiare tutto mando anche qui l'username anche se inutile

                    incomingClient->writeData(response);
                    instance->db.close();
                    //mando la lista dei file
                    this->sendFileList(incomingClient);
                }
                else {
                    //già ho notificato al client che la pwd è errata
                    return;
                }
            }
            else {
                qDebug() << "Username not valid\n";
                message = "WRONG USERNAME OR PASSWORD";
                response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                incomingClient->writeData(response);
                instance->db.close();
                return;
            }
        }
        else {
            qDebug() << "SELECT COUNT(*) NOT executed: " << query.lastError() << "\n";
            sendError(incomingClient);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(incomingClient);
        return;
    }
    return;
}

void DBInteraction::logout(ClientManager* client) {
    //cancella semplicemente l'utente dalle strutture interne(mappe) locali del server
    if (!instance->isUserLogged(client)) {
        return;
    }
    for (File* f : instance->files) {
        if (f->getUsers().contains(client)) {
            f->removeUser(client);
        }
    }
    instance->activeusers.removeOne(client);
}

void DBInteraction::createFile(QString filename, ClientManager* client) {

    QSqlQuery query;
    int cnt = 0;
    int fileId = 1;
    QString path = QDir::currentPath().append("/").append("files").append("/");// + "\\files\\";
    QString message;
    QByteArray response;

    if (!instance->isUserLogged(client)) {
        return;
    }
    QString username = client->getUsername();
    int userId = client->getId();

    if (instance->db.open()) {

        query.prepare("SELECT COUNT(*) FROM files WHERE UserId = (:userid) AND FileName = (:filename)");
        query.bindValue(":userid", userId);
        query.bindValue(":filename", filename);
        if (query.exec()) {
            if (query.next()) {
                cnt = query.value(0).toInt();
            }
            if (cnt != 0) {
                //l'utente non puÃ² avere 2 file con lo stesso nome
                qDebug() << "ERROR: THE FILE ALREADY EXISTS!\n";
                message = "THE FILE ALREADY EXISTS";  // NON COMPARE IL MESSAGGIO !!!!!!!!!!!!!!
                response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                client->writeData(response);
                instance->db.close();
                return;
            }
            else {
                //il file non esiste, quindi posso crearlo

                QSqlQuery query2;
                query2.prepare("SELECT MAX(fileId) FROM files"); //l'id del file Ã¨ un intero crescente: uso MAX perchÃ¨ l'id deve essere unico per file differenti ma uguale per file uguali, quindi se avessi:
                                                             // 1 prova.txt Mattia
                                                             // 1 prova.txt Ilio   (Ilio e Mattia condividono il file prova.txt)
                                                             // 2 file.txt Ilio
                                                             // 2 file.txt Mattia  --> COUNT ritornerebbe 4 ma io sono arrivato all'id 2, quindi il prossimo dovrebbe essere 3!!
                if (query2.exec()) {
                    if (query2.next()) {
                        fileId = query2.value(0).toInt() + 1;
                    }
                    qDebug() << "fileId: " << fileId << "\n";


                    QStringList parts = filename.split('.');
                    filename = parts.at(0);
                    qDebug() << "filename: " << filename << "\n";

                    QString text_path = path.append(username).append("/").append(filename).append(".txt"); //  esempio --> currdir/files/ilio/prova.txt

                    //qDebug()<< "Path: " << path << "\n";
                    /*
                    QString user_directory_path(path.append(username));
                    if(!QDir(user_directory_path).exists()){
                        //creo la cartella files/username
                        QDir().mkdir(user_directory_path);
                    }*/

                    QSqlQuery query3;
                    query3.prepare("INSERT INTO files(FileId, FileName, userid, Path, SiteCounter) VALUES ((:fileId), (:filename), (:userid), (:path), 0)");
                    query3.bindValue(":fileId", fileId);
                    query3.bindValue(":filename", filename);
                    query3.bindValue(":userid", userId);
                    query3.bindValue(":path", text_path);

                    if (query3.exec()) {
                        /*
                        QFile file(text_path);
                        if(file.open(QIODevice::WriteOnly)){
                            QTextStream stream(&file);
                            stream << text_path;
                            file.close();
                        }*/ 
                        response = Serialize::fromObjectToArray(Serialize::newFileSerialize(filename, fileId, NEWFILE));
                        client->writeData(response);
                        qDebug() << "sono qui\n";

                        //instance->files.insert(fileId, newfile);
                        //instance->openFile(fileId, client);
                        instance->db.close();

                    }

                    else {
                        qDebug() << "INSERT failed: " << query3.lastError() << "\n";
                        sendError(client);
                        instance->db.close();
                        return;
                    }

                }
                else {
                    qDebug() << "SELECT COUNT(fileId) failed: " << query2.lastError() << "\n";
                    sendError(client);
                    instance->db.close();
                    return;
                }
            }
        }
        else {
            qDebug() << "SELECT COUNT(*) failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
        instance->db.close();
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return;
    }
    return;
}

void DBInteraction::sendFileList(ClientManager* client) {

    if (!instance->isUserLogged(client)) {
        return;
    }

    int userid = client->getId();

    if (instance->db.open()) {
        QSqlQuery query;
        //QJsonArray files; // la lista Ã¨ vuota?
        QByteArray response;
        QMap<int, QString> fileList;


        query.prepare("SELECT FileName, FileId FROM files WHERE UserId = (:userid)");
        query.bindValue(":userid", userid);


        if (query.exec()) {
            bool atLeastOne = false;

            while (query.next()) {
                //per ogni file creo un jsonObject contenente nome del file e id

                QString filename = query.value("FileName").toString();
                int fileId = query.value("FileId").toInt();

                fileList.insert(fileId, filename);

                atLeastOne = true;
            }

            if (atLeastOne) {
                response = Serialize::fromObjectToArray(Serialize::FileListSerialize(fileList, SEND_FILES));
                client->writeData(response);
            }
            else {
                qDebug() << "no files\n";
            }
            instance->db.close();
        }
        else {
            qDebug() << "SELECT failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return;
    }
}

void DBInteraction::openFile(int fileId, ClientManager* client) {

    File* f = nullptr;
    QString message;
    QByteArray response;
    int siteCounter = 0;
    bool is_in_RAM = false;

    if (!instance->isUserLogged(client)) {
        return;
    }
    qDebug() << "fileId da aprire: " << fileId << "\n";
    if (instance->files.contains(fileId)) {
        //il file Ã¨ gia in RAM

        if (instance->files.value(fileId)->getUsers().contains(client)) {
            qDebug() << "file already opened!\n";
            message = "file already opened!";
            response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
            client->writeData(response);
            return;
        }
        is_in_RAM = true;
    }

    //selezione del siteCounter
    if (instance->db.open()) {
        QSqlQuery query;
        int userid = client->getId();
        query.prepare("SELECT siteCounter FROM files WHERE userid = (:userid) AND fileId = (:fileid)");
        query.bindValue(":userid", userid);
        query.bindValue(":fileid", fileId);
        if (query.exec()) {
            if (query.next()) {
                siteCounter = query.value("siteCounter").toInt();
                qDebug() << "siteCounter = " << siteCounter << "\n";
            }
        }
        else {
            qDebug() << "SELECT siteCounter failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }

    }

    if (is_in_RAM) {
        //il file Ã¨ gia in RAM

        f = instance->getFile(fileId);
        f->addUser(client);
        qDebug() << "ho inviato il file che era gia in RAM\n";
        response = Serialize::fromObjectToArray(Serialize::siteCounterSerialize(fileId, siteCounter, SITECOUNTER));
        client->writeData(response);

        // sendSuccess(client);
    }
    else {
        //cercare il file nel DB
        if (instance->db.open()) {
            QString path;
            QSqlQuery query;
            int userid = client->getId();
            query.prepare("SELECT path FROM files WHERE userid = (:userid) AND fileId = (:fileid)");
            query.bindValue(":userid", userid);
            query.bindValue(":fileid", fileId);

            if (query.exec()) {
                if (query.next()) {
                    path = QString(query.value("path").toString());
                    f = new File(fileId, path);
                    //f->addUser(client);
                    instance->files.insert(fileId, f);
                    instance->files.value(fileId)->addUser(client);
                    qDebug() << "user added!\n";
                    //sendSuccess(client);
                    response = Serialize::fromObjectToArray(Serialize::siteCounterSerialize(fileId, siteCounter, SITECOUNTER));
                    client->writeData(response);
                    instance->db.close();


                }
                else {
                    qDebug() << "this file does not exist!\n";
                    message = "this file does not exist!";
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                    client->writeData(response);
                    instance->db.close();
                    return;

                }
            }
            else {
                qDebug() << "SELECT path failed: " << query.lastError() << "\n";
                sendError(client);
                instance->db.close();
                return;
            }
        }
        else {
            qDebug() << "DB not opened!!\n";
            sendError(client);
            return;
        }

    }
}

void DBInteraction::closeFile(int fileId, int siteCounter, ClientManager* client) {
    //per ogni utente che richiede la chiusura del file verrÃ  fatta una removeUser
    QByteArray response;
    QString message;
    File* f;

    if (!instance->isUserLogged(client)) {
        return;
    }
    qDebug() << "fileId da chiudere: " << fileId << "\n";
    qDebug() << "sitecounter del file da chiudere: " << siteCounter << "\n";

    if (!instance->files.contains(fileId)) { // se il file non si trova nella mappa di file attivi in quel momento vuol dire che o nessuno lo sta utilizzando e quindi Ã¨ giÃ  chiuso oppure che non esiste
        qDebug() << "this file does not exist or it is not opened!\n";
        message = "this file does not exist or it is not opened!";
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        client->writeData(response);
        return;
    }

    if (instance->files.value(fileId)->getUsers().contains(client)) {
        //se il file risulta aperto dall'utente allora lo puÃ² chiudere

        if (instance->files.value(fileId)->is_file_shared() && !instance->files.value(fileId)->getRUsers().contains(client) && f->isModifiedName()) {
            instance->files.value(fileId)->addRUser(client); //serve per la rinomina 
            qDebug() << "client aggiunto alla lista di utenti che avranno un file da rinominare\n";
        }

        //aggiornamento del siteCounter
        if (instance->db.open()) {
            QSqlQuery query;
            int userid = client->getId();
            qDebug() << "userId dell'utente che vuole chiudere il file: " << userid << "\n";
            query.prepare("UPDATE files SET SiteCounter = (:sitecounter) WHERE fileId = (:fileid) AND userid = (:userid)");
            query.bindValue(":sitecounter", siteCounter);
            query.bindValue(":fileid", fileId);
            query.bindValue(":userid", userid);

            if (!query.exec()) {
                qDebug() << "UPDATE sitecounter failed: " << query.lastError() << "\n";
                sendError(client);
                instance->db.close();
                return;
            }
            qDebug() << "update riuscita\n";
            instance->db.close();
        }
        else {
            qDebug() << "DB not opened!!\n";
            sendError(client);
            return;
        }

        f = instance->files.value(fileId);
        f->removeUser(client);

        if (!f->thereAreUsers()) {
            // in questo momento che il file non Ã¨ aperto da nessuno, controllo se uno degli utenti ha richiesto di rinominare il file e nel caso lo rinomino
            //se non ho piÃ¹ utenti attivi che lavorano sul file allora lo rimuovo dalla mappa (no dal DB, quindi il file esiste ancora!!)

            if (f->isModifiedName()) {
                QString username = client->getUsername();
                QString oldName;
                QString newName = f->getNewName();
                QString oldPath = f->getPath();
                int i;

                oldName = instance->changeFileName(oldPath, newName, fileId, client);

                if (oldName != nullptr) {
                    response = Serialize::fromObjectToArray(Serialize::renameFileSerialize(oldName, newName, RENAME));
                    for (i = 0; i < instance->files.value(fileId)->getRUsers().size(); i++) {
                        f->getRUsers().at(i)->writeData(response);
                        //non rimuovo tutti questi client perche tanto tra poco f verrà eliminato 
                    }
                    //client->writeData(response);
                }
            }
            instance->files.remove(fileId);//nessuno sta più usando il file
        }
    }
    return;
}

void DBInteraction::deleteFile(int fileId, ClientManager* client) {
    //per ogni utente che richiede la cancellazione verrÃ  cancellata nel DB la riga corrispondente
    QByteArray response;
    QString message;
    QSqlQuery query;
    QString path;
    bool last = false;


    if (!instance->isUserLogged(client)) {
        return;
    }
    QString username = client->getUsername();
    int userid = client->getId();


    if (instance->files.contains(fileId)) {
        //qualcuno sta usando il file
        if (instance->files.value(fileId)->getUsers().contains(client)) {
            //il file è aperto dal nostro utente (MA PUO DARSI CHE NON SIA IL SOLO CHE LO STA USANDO!!)
            instance->files.value(fileId)->removeUser(client);
        }
    }

    if (instance->db.open()) {
        QSqlQuery query2;
        int cnt = -1;

        query2.prepare("SELECT COUNT(*) FROM files WHERE fileId = (:fileid)"); //nel caso fosse rimansto solamente 1 file con quel id vuol dire che con la delete che farò dopo non ci sarà piu nel DB, allora lo posso cancellare dalla tabella
        query2.bindValue(":fileid", fileId);

        if (query2.exec()) {
            if (query2.next()) {
                cnt = query2.value(0).toInt();
                if (cnt == 1) {
                    //il file non esisterà più nel DB, allora lo elimino anche dalla cartella files
                    qDebug() << "file da eliminare dalla cartella files\n";
                    QSqlQuery query3;
                    last = true;
                    
                    query3.prepare("SELECT path FROM files WHERE fileid = (:fileid) AND userid = (:userid)");
                    query3.bindValue(":userid", userid);
                    query3.bindValue(":fileid", fileId);
                    if (query3.exec()) {
                        if (query3.next()) {
                            path = QString(query3.value("path").toString());
                        }
                    }
                    else {
                        qDebug() << "SELECT path failed: " << query3.lastError() << "\n";
                        sendError(client);
                        instance->db.close();
                        return;
                    }
                }
            }
            else{
                qDebug() << "SELECT COUNT(*) failed: " << query2.lastError() << "\n";
                sendError(client);
                instance->db.close();
                return;
            }
        }
        query.prepare("DELETE FROM files WHERE fileId = (:fileid) AND userid = (:userid)");
        query.bindValue(":userid", userid);
        query.bindValue(":fileid", fileId);
        if (query.exec()) {
            qDebug() << "file deleted!!\n";
            sendSuccess(client);
            instance->db.close();
            if (last) {
                qDebug() << "il file verrà anche rimosso dalla cartella perchè eliminato da tutti gli utenti\n";
                QFile::remove(path);
            }
        }
        else {
            qDebug() << "DELETE failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return;
    }
    return;
}

QString DBInteraction::changeFileName(QString oldPath, QString newName, int fileId, ClientManager* client){
    QString newPath;
    int i;
    QStringList oldPathList = oldPath.split("/");
    for (i = 0; i < oldPathList.size() - 1; i++) {
        //copio il precedente path eliminando l'ultima stringa che rappresenta il nome del vecchio file
        newPath.append(oldPathList.at(i)).append("/");
    }
    QString oldName = oldPathList.at(i);
    //qDebug() << "old filename: " << oldName << "\n";

    QStringList parts = newName.split('.');
    newName = parts.at(0);
    newPath.append(newName).append(".txt");
    //qDebug() << "new filename: " << newName << "\n";
    //qDebug() << "new path: " << newPath << "\n";

    QFile::rename(oldPath, newPath);

    if (instance->db.open()) {
        QSqlQuery query;
        query.prepare("UPDATE files SET filename = (:newname), path = (:newpath) WHERE FileId = (:fileid) ");
        query.bindValue(":newname", newName);
        query.bindValue(":newpath", newPath);
        query.bindValue(":fileid", fileId);

        if (query.exec()) {
            
            return oldName.split(".").at(0);
        }
        else {
            qDebug() << "UPDATE failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return nullptr;
        }
        instance->db.close();
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return nullptr;
    }



}

void DBInteraction::renameFile(int fileId, QString newName, ClientManager* client) {
    // per ogni file bisogna cambiare il nome all'interno del DB e il nome del path (anche nel DB), cosa fatta nella closeFile
    QByteArray response;
    QString message;
    QSqlQuery query;
    File* f;

    if (!instance->isUserLogged(client)) {
        return;
    }

    if (!instance->files.contains(fileId)){ ///*|| (instance->files.value(fileId)->getUsers().contains(client) && instance->files.value(fileId)->getUsers().size() == 1)*/) {
        //nessuno sta lavorando sul file (non vuol dire che sia l'unico a possederlo!!)
        QSqlQuery query, query2;
        int userid = client->getId();


        if (instance->db.open()) {
            query.prepare("SELECT path FROM files WHERE fileid = (:fileid) AND userid = (:userid)");
            query.bindValue(":fileid", fileId);
            query.bindValue(":userid", userid);

            if (query.exec() ) {
                if (query.next()) {
                    QString oldPath = QString(query.value("path").toString());

                    //lista di utenti che possiedono il file ma che non lo stanno usando
                    query2.prepare("SELECT UserId FROM files WHERE FileId = (:fileid)");
                    query2.bindValue(":fileid", fileId);

                    if (query2.exec()) {
                        int i;
                        ClientManager* user;
                        QString oldName;
                        QList<int> users;

                        qDebug() << "fileId da rinominare: " << fileId << "\n";

                        while (query2.next()) {

                            qDebug() << "size activeUsers: " << instance->activeusers.size() << "\n";
                            qDebug() << "query result: " << query2.value("UserId").toInt() << "\n";
                            users.append(query2.value("UserId").toInt());

                        }
                        instance->db.close();
                        for (int userid : users) {
                            for (i = 0; i < instance->activeusers.size(); i++) {

                                if (instance->activeusers.at(i)->getId() == userid) {

                                    user = instance->activeusers.at(i);
                                    oldName = instance->changeFileName(oldPath, newName, fileId, user);
                                    response = Serialize::fromObjectToArray(Serialize::renameFileSerialize(oldName, newName, RENAME));
                                    user->writeData(response);

                                }
                            }
                        }

                    }
                    else {
                        qDebug() << "SELECT userid failed: " << query2.lastError() << "\n";
                        sendError(client);
                        instance->db.close();
                        return;
                    
                    }
                }
            }
            else {
                qDebug() << "SELECT path failed: " << query.lastError() << "\n";
                sendError(client);
                instance->db.close();
                return;
            }
        }
        else{
            qDebug() << "DB not opened!!\n";
            sendError(client);
            return;
        }
    }
    else{
        //il file è aperto da altri utenti
        // nel caso in cui il file sia condiviso tra piÃ¹ utenti, uno di questi vuole cambiare il nome mentre gli altri hanno ancora il file aperto e lo stanno modificando, come faccio?? risposta in closeFile!!
        qDebug() << "il file è ancora aperto prima del cambio nome\n";
        f = instance->files.value(fileId);
        if (f->getUsers().size() > 1) f->setSharedFile();
        f->modifyName(newName); //tengo traccia dell'ultimo client che ha richiesto un cambio nome(ogni utente aggiorna la stringa newName contenuta nel file, quindi quella che trovo alla fine sarÃ  l'ultima)
        f->addRUser(client);
        sendSuccess(client);
    }
}

void DBInteraction::getURIToShare(int fileid, ClientManager* client) {
    QByteArray response;
    QString message;
    QString URI;
    if (!instance->isUserLogged(client)) {

        return;
    }
    if (instance->files.contains(fileid)) {
        URI = instance->files.value(fileid)->getPath();
        response = Serialize::fromObjectToArray(Serialize::URISerialize(URI, SENDURI));
        client->writeData(response);
    }
    else {
        qDebug() << "file not existing\n";
        sendError(client);
    }
}

void DBInteraction::SharedFileAcquisition(QString URI, ClientManager* client) {

    QByteArray response;
    QString message;

    if (!instance->isUserLogged(client)) {
        return;
    }

    qDebug() << "URI: " << URI << "\n";
    //recupero l'ID del file;
    //aggiungo nel DB la riga corrispondente al nuovo utente;
    //chiamo la open
    if (instance->db.open()) {
        QSqlQuery query;

        query.prepare("SELECT fileId, fileName FROM files WHERE path = (:path)"); // dovrebbe essere univoca la risposta: i path sono costruiti in modo tale da non poterne avere 2 uguali, quindi la query mi restituisce 1 valore solo
        query.bindValue(":path", URI);

        if (query.exec()) {
            int fileId;
            QString filename;
            if (query.next()) {

                fileId = query.value("fileId").toInt();
                filename = query.value("fileName").toString();

            }
            else {
                qDebug() << "this file does not exist!\n";
                message = "this file does not exist!";
                response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                client->writeData(response);
                instance->db.close();
                return;
            }
            QSqlQuery query2;
            QString username = client->getUsername();
            int userid = client->getId();
            qDebug() << "username: " << username << "\n";
            qDebug() << "userid: " << userid << "\n";

            query2.prepare("INSERT INTO files(fileId, FileName, userid, Path, siteCounter) VALUES ((:fileId), (:filename), (:userid), (:path), 0)");
            query2.bindValue(":filename", filename);
            query2.bindValue(":fileId", fileId);
            query2.bindValue(":userid", userid);
            query2.bindValue(":path", URI);
            if (query2.exec()) {

                //sendSuccess(client);
                QMap<int, QString> id_file;
                id_file.insert(fileId, filename);
                QByteArray message = Serialize::fromObjectToArray(Serialize::FileListSerialize(id_file, SEND_FILES));
                client->writeData(message);
                instance->db.close();
                //Ã¨ meglio aspettare una richiesta specifica dal client per aprire il file perchÃ¨ se il client fosse lento si perderebbe i messaggi
                


            }
            else {
                qDebug() << "INSERT failed: " << query2.lastError() << "\n";
                sendError(client);
                instance->db.close();
                return;
            }
        }
        else {
            qDebug() << "SELECT failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return;
    }
}

void DBInteraction::changePassword(QString oldPassword, QString newPassword, ClientManager* client) {

    if (!instance->isUserLogged(client)) {
        return;
    }

    if (checkPassword(oldPassword, client)) {
        if (instance->db.open()) {
            QByteArray response;
            QString message;
            QSqlQuery query;
            QByteArray salted_pwd;
            QString hashed_pwd;
            QString salt;
            int userid = client->getId();

            salt = instance->generateRandomString(newPassword.size());
            salted_pwd = newPassword.append(salt).toUtf8();
            hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));
            qDebug() << "new password: " << hashed_pwd << "\n";

            query.prepare("UPDATE users SET Password = (:newpassword), Salt = (:salt) WHERE userid = (:userid)");
            query.bindValue(":newpassword", hashed_pwd);
            query.bindValue(":salt", salt);
            query.bindValue(":userid", userid);

            if (query.exec()) {
                sendSuccess(client);
                instance->db.close();
            }
            else {
                qDebug() << "UPDATE failed: " << query.lastError() << "\n";
                sendError(client);
                instance->db.close();
                return;
            }
        }
        else {
            qDebug() << "DB not opened!!\n";
            sendError(client);
            return;
        }
    }
}

/*
* void DBInteraction::changeUsername(QString newUsername, ClientManager* client) {

    if (instance->db.open()) {
        int userid = client->getId();
        QByteArray response;
        QString message;
        QSqlQuery query;

        query.prepare("UPDATE users SET username = (:newname) WHERE userid = (:userid)");
        query.bindValue(":newname", newUsername);
        query.bindValue(":userid", userid);

        if (query.exec()) {
            client->setUsername(newUsername);
            sendSuccess(client);
            instance->db.close();
        }
        else {
            qDebug() << "UPDATE failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return;
    }
}


void DBInteraction::changeEmail(QString newEmail, ClientManager* client) {
    QByteArray response;
    QString message;

    if (!is_email_valid(newEmail)) {
        qDebug() << "not a valid email!\n";
        message = "EMAIL IS NOT VALID!";
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        client->writeData(response);
        return;
    }



    if (instance->db.open()) {
        int userid = client->getId();

        QSqlQuery query;


        query.prepare("UPDATE users SET email = (:newEmail) WHERE userid = (:userid)");
        query.bindValue(":newEmail", newEmail);
        query.bindValue(":userid", userid);

        if (query.exec()) {
            client->setEmail(newEmail);
            sendSuccess(client);
            instance->db.close();
        }
        else {
            qDebug() << "UPDATE failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return;
    }
}


void DBInteraction::changeProfilePic(QString profileImage, ClientManager* client) {
    QString username = client->getUsername();
    QByteArray response;
    QString message;
    QSqlQuery query;

    QString path(QDir::currentPath() + "/ImmaginiProfilo/" + username + "_profileImage.txt");

    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    QFile file(path);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << profileImage;
        file.close();
    }
    else {
        qDebug("image not saved!!\n");
        message = "image not saved!\n";
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        client->writeData(response);
        instance->db.close();
        return;
    }
    if (instance->db.open()) {
        QSqlQuery query;
        int userid = client->getId();

        query.prepare("UPDATE users SET profileImage = (:profileImage) WHERE userid = (:userid)");
        query.bindValue(":profileImage", path);
        query.bindValue(":userid", userid);


        if (query.exec()) {
            //success
            qDebug("image changed!\n");
            sendSuccess(client);
            instance->db.close();
        }
        else {
            qDebug("UPDATE failed\n");
            qDebug() << query.lastError();
            sendError(client);
            instance->db.close();
            return;
        }

    }
    else {
        qDebug() << "DB not opened!!\n";
        sendError(client);
        return;
    }

}

*/

void DBInteraction::changeProfile(QString oldUsername, QString newUsername, QString oldEmail, QString newEmail, QString newImage, ClientManager* client) {
    if (!instance->isUserLogged(client)) {
        return;
    }

    QString username = client->getUsername();
    QByteArray response;
    QString message;
    QSqlQuery query;

    QString path(QDir::currentPath() + "/ImmaginiProfilo/" + username + "_profileImage.txt");

    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    qDebug() << oldUsername << "-->" << newUsername<< "\n";
    qDebug() << oldEmail << "-->" << newEmail << "\n";


    if ( (is_email_valid(newEmail) && is_username_unique(newUsername)) ||  // vengono cambiati sia email che username
         (is_email_valid(newEmail) && oldUsername.compare(newUsername) == 0) || // viene cambiata solo l'email
         (oldEmail.compare(newEmail) == 0 && is_username_unique(newUsername)) || //viene cambiato solo l'username
         (oldEmail.compare(newEmail) == 0 && oldUsername.compare(newUsername) == 0 ) ) { //viene cambiata solamente l'immagine del profilo

        QFile file(path);

        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << newImage;
            file.close();
        }
        else {
            qDebug("image not saved!!\n");
            message = "image not saved!\n";
            response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
            client->writeData(response);
            instance->db.close();
            return;
        }


        if (instance->db.open()) {
            QSqlQuery query;
            int userid = client->getId();

            query.prepare("UPDATE users SET username = (:newname), email = (:newEmail), profileImage = (:profileImage) WHERE userid = (:userid)");
            query.bindValue(":newname", newUsername);
            query.bindValue(":newEmail", newEmail);
            query.bindValue(":profileImage", path);
            query.bindValue(":userid", userid);


            if (query.exec()) {
                //success
                qDebug("Profile updated!\n");
                client->setUsername(newUsername);
                client->setEmail(newEmail);
                QColor userColor = client->getColor();
                response = Serialize::fromObjectToArray(Serialize::changeProfileResponseSerialize(true, newUsername, newEmail, newImage, "", SERVER_ANSWER));

                client->writeData(response);
                //sendSuccess(client);
                instance->db.close();
            }
            else {
                qDebug("UPDATE failed\n");
                qDebug() << query.lastError();
                sendError(client);
                instance->db.close();
                return;
            }
        }
        else {
            qDebug() << "DB not opened!!\n";
            sendError(client);
            return;
        }
    }
    else {
        message = "username or email (or both) is/are not valid\n";
        response = Serialize::fromObjectToArray(Serialize::changeProfileResponseSerialize(false, "", "", "", message, SERVER_ANSWER));
        client->writeData(response);
        return;
    }

    /*
    if (!newUsername.isEmpty()) instance->changeUsername(newUsername, client);
    if (!newEmail.isEmpty()) instance->changeEmail(newEmail, client);
    if (!newImage.isEmpty()) instance->changeProfilePic(newImage, client);
    */

}

void DBInteraction::forwardMessage(ClientManager* user, QJsonObject obj, QByteArray data)
{
    //qDebug()<< data;
    QPair<int, Message> fileid_message = Serialize::messageUnserialize(obj);

    //File *f = instance->getFile(fileid_message.first);
    File* f = instance->getFile(fileid_message.first);// debug only
    f->messageHandler(user, fileid_message.second, data);
}

File* DBInteraction::getFile(int fileid) {
    return instance->files.value(fileid);
}

bool DBInteraction::isUserLogged(ClientManager* client) {

    if (!instance->activeusers.contains(client)) {
        qDebug() << "user not authorized!\n";
        //anche se non Ã¨ attivo, l'utente ha comunque un socket
        sendError(client);
        return false;
    }
    return true;


}

QByteArray DBInteraction::intToArray(qint64 source) {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

//generazione casuale di un colore ed inserimento nella lista cosÃ¬ che non venga ripetuto
QColor DBInteraction::generateRandomColor(int userId) {
    QColor newColor;
    do {
        newColor = QColor::fromRgb(QRandomGenerator::global()->generate());
    } while (colorPresent(newColor));
    m_colorPerUser.insert(userId, newColor);
    return newColor;
}

bool DBInteraction::colorPresent(QColor color) {
    for (auto it = m_colorPerUser.begin(); it != m_colorPerUser.end(); it++) {
        if (it.value() == color) return true;
    }
    return false;
}
