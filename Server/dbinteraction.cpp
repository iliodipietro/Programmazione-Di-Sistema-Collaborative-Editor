#include "dbinteraction.h"
#include <QDir>
#include <QDataStream>
#include <QRandomGenerator>

DBInteraction *DBInteraction::instance = nullptr;

DBInteraction::DBInteraction(){}

DBInteraction* DBInteraction::startDBConnection(){
    bool err = false;
    QString path = QDir::currentPath().append("/project.sqlite"); //project.db";scegliere path in cui salvare il DB

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
                                  "Nickname VARCHAR(255) NOT NULL,"
                                  "Salt     VARCHAR(255),"
                                  "ProfileImage VARCHAR(255) NOT NULL);");

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
        instance->db.close();
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
/*void DBInteraction::sendMessage(QTcpSocket *socket, QByteArray obj){
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

}*/

void DBInteraction::sendError(ClientManager* client){
    QString message;
    QByteArray response;

    message = "SERVER ERROR";
    response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
    client->writeData(response);
 //   sendMessage(socket, response);
    return;
}

void DBInteraction::sendSuccess(ClientManager* client){
    QString message;
    QByteArray response;

    message = "OK";
    response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, message, SERVER_ANSWER));
    client->writeData(response);
    //sendMessage(socket, response);
    return;
}

QString DBInteraction::computeHashPassword(QString password){
    QString hashed_pwd;
    QString salt;
    QByteArray salted_pwd;

    salt = instance->generateRandomString(password.size());
    salted_pwd = password.append(salt).toUtf8();
    hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));
    return hashed_pwd;
}

bool DBInteraction::checkPassword(QString password,  ClientManager* client){

    if(instance->db.open()){
        QSqlQuery query;
        QByteArray salted_pwd;
        QString hashed_pwd;
        QString salt;
        QString message;
        QByteArray response;
        QString username = client->getUsername();

        qDebug()<<"checking password...\n";

        query.prepare("SELECT Password, userid, Salt, profileImage FROM users WHERE Username = (:username)");
        query.bindValue(":username", username);
        if (query.exec()) {

            if(query.next()){

                salt = QString(query.value("Salt").toString());
                salted_pwd = password.append(salt).toUtf8();
                hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));

                if(hashed_pwd.compare(QString(query.value("Password").toString())) == 0){
                    instance->db.close();
                    return true;
                }
                else {
                    qDebug()<< "Password not valid!!\n";
                    message = "WRONG USERNAME OR PASSWORD";
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                    client->writeData(response);
                   // sendMessage(socket, response);
                    instance->db.close();
                    return false;
                }
            }
            else {
                sendError(client);
                instance->db.close();
                return false;
            }
        }
        else {
            qDebug()<< "SELECT Password failed: "<< query.lastError()<<"\n";
            sendError(client);
            instance->db.close();
            return false;
        }
    }
    else {
        qDebug()<<"DB not opened!!\n";
        sendError(client);
        return false;
    }
}

void DBInteraction::registration(QString username, QString password, QString nickname, QString profileImage, ClientManager* incomingClient){

    QSqlQuery query, query2;
    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QByteArray response;
    QString message;
    int cnt2 = 0;
    int cnt = 0;
    int userid = 1;

    if(instance->db.open()){ //bisogna aprire la connessione al db prima altrimenti non funziona


        /*CONTROLLO UNICITà NICKNAME E USERNAME*/

        query2.prepare("SELECT COUNT(*) FROM users WHERE Nickname = (:nickname)");
        query2.bindValue(":nickname", nickname);

        if(query2.exec()){
            if(query2.next()){
                cnt2 = query.value(0).toInt();
                if(cnt2 > 0){
                    qDebug("nickname already exists\n");
                    message = "Nickname already used";
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                    incomingClient->writeData(response);
                    //sendMessage(socket, response);
                    instance->db.close();
                    return;
                }
            }
        }
        else {
            qDebug()<< "SELECT COUNT2 query failed!"<<query.lastError()<<"\n";
            sendError(incomingClient);
            instance->db.close();
            return;
        }

        query.prepare("SELECT COUNT(*) FROM users WHERE Username = (:username)");
        query.bindValue(":username", username); // no matching member function for call to 'bindValue' --> risolto con #incliude <QVariant>

        if(query.exec()){
            if(query.next()){
                cnt = query.value(0).toInt();
                if(cnt > 0){
                    qDebug("username already exists\n");
                    message = "Username already used";
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                    incomingClient->writeData(response);
                    //sendMessage(socket, response);
                    instance->db.close();
                    return;
                }
            }
        }
        else {
            qDebug()<< "SELECT COUNT query failed!"<<query.lastError()<<"\n";
            sendError(incomingClient);
            instance->db.close();
            return;
        }
        /*FINE CONTROLLO*/

        /*INSERIMENTO DATI NEL DB*/
        qDebug("insertion...\n");
        salt = instance->generateRandomString(password.size());
        salted_pwd = password.append(salt).toUtf8();
        hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));
        qDebug()<<"new password: "<< hashed_pwd<<"\n";

        QSqlQuery query2;
        query2.prepare("SELECT COUNT(UserId) FROM users"); //l'id dell'utente è un intero crescente
        if(query2.exec()){
            if(query2.next()){
                userid = query2.value(0).toInt();
            }
        }
        else{
            qDebug()<< "SELECT COUNT(UserId) query failed!"<<query.lastError()<<"\n";
            sendError(incomingClient);
            instance->db.close();
            return;
        }

        QString images_directory_path(QDir::currentPath() + "\\ImmaginiProfilo\\");
        if(!QDir(images_directory_path).exists()){          //if(!QFile::exists(images_directory_path)){
            //creo la cartella ImmaginiProfilo
            QDir().mkdir(images_directory_path);
        }

        QString path(QDir::currentPath() + "/ImmaginiProfilo/" + username + "_profileImage.txt");
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            stream << profileImage;
            file.close();
        }
        else{
            qDebug("image not saved!!\n");
            message = "image not saved!\n";
            response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
            incomingClient->writeData(response);
            //sendMessage(socket, response);
            instance->db.close();
            return;
        }

        QString user_directory_path(QDir::currentPath() + "/" + username + "/");
        if(!QDir(user_directory_path).exists()){          //if(!QFile::exists(images_directory_path)){
            //creo la cartella ImmaginiProfilo
            QDir().mkdir(user_directory_path);
        }

        qDebug()<<"userid: "<< userid<<"\n";

        QSqlQuery query3;
        query3.prepare("INSERT INTO users(username, userid, password, nickname, salt, profileImage) VALUES ((:username),(:userid), (:password), (:nickname), (:salt), (:profileImage))");
        query3.bindValue(":username", username);
        query3.bindValue(":userid", userid);
        query3.bindValue(":password", hashed_pwd);
        query3.bindValue(":nickname", nickname);
        query3.bindValue(":salt", salt);
        query3.bindValue(":profileImage", path);

        if(query3.exec()){
            //success
            qDebug("new user added!\n");
            sendSuccess(incomingClient);
            instance->db.close();
        }
        else{
            qDebug("INSERT failed\n");
            qDebug()<<query3.lastError();
            sendError(incomingClient);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug()<<"DB not opened!!\n";
        sendError(incomingClient);
        return;
    }
    return;
}

void DBInteraction::login(QString username, QString password, ClientManager* incomingClient){

    QSqlQuery query;
    QByteArray salted_pwd;
    QString hashed_pwd;
    QString salt;
    QString profileImage, profileImagePath;
    QByteArray response, response_ok;
    QString message;
    int cnt = 0;
    int userid;

    if(instance->db.open()){

        query.prepare("SELECT COUNT(*),* FROM users WHERE Username = (:username)");
        query.bindValue(":username", username);
        if(query.exec()){

            if(query.next()){
                cnt = query.value(0).toInt();
            }
            if(cnt == 1){

            incomingClient->setUsername(username);

                if(checkPassword(password, incomingClient)){
                    //success
                    userid = query.value("UserId").toInt();
                    profileImagePath = query.value("ProfileImage").toString();
                    QFile file(profileImagePath);

                    if(file.open(QIODevice::ReadOnly)){
                        QTextStream stream(&file);
                        profileImage.append(stream.readAll());
                        file.close();
                    }
                    else{
                        qDebug("image not available!!\n");
                        message = "image not available!\n";
                        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                        incomingClient->writeData(response);
                        //sendMessage(socket, response);
                        instance->db.close();
                        return;
                    }

                    incomingClient->setId(userid);
                    instance->activeusers.push_back(incomingClient);
                    QColor userColor = instance->generateRandomColor(userid);
                    incomingClient->setColor(userColor);
                   // instance->users.insert(username, new ClientManager(userid,socket));
                    response = Serialize::fromObjectToArray(Serialize::responseSerialize(true, profileImage, SERVER_ANSWER, userid, userColor));
                    incomingClient->writeData(response);
                    instance->db.close();
                    //mando la lista dei file
                    this->sendFileList(incomingClient);
                   // sendMessage(socket, response);
                }
                else {
                    return;
                }
            }
            else {
                qDebug()<< "Username not valid\n";
                message = "WRONG USERNAME OR PASSWORD";
                response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                incomingClient->writeData(response);
                instance->db.close();

               // sendMessage(socket, response);

                return;
            }
        }
        else{
            qDebug()<< "SELECT COUNT(*) NOT executed: "<< query.lastError()<<"\n";
            sendError(incomingClient);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug()<<"DB not opened!!\n";
        sendError(incomingClient);
        return;
    }
    return;
}

void DBInteraction::logout(ClientManager* client){
    //cancella semplicemente l'utente dalle strutture interne(mappe) locali del server
    if(!instance->isUserLogged(client)){
        return;
    }
    for(File* f : instance->files){
        if(f->getUsers().contains(client)){
            f->removeUser(client);
        }
    }
    //sendSuccess(client); // ??
    instance->activeusers.removeOne(client);

}

void DBInteraction::createFile(QString filename, ClientManager* client){

    QSqlQuery query;
    int cnt = 0;
    int fileId = 0;
    QString path = QDir::currentPath();
    QString message;
    QByteArray response;

    if(!instance->isUserLogged(client)){
        return;
    }
    QString username = client->getUsername();

    if(instance->db.open()){

        query.prepare("SELECT COUNT(*) FROM files WHERE UserName = (:username) AND FileName = (:filename)");
        query.bindValue(":username", username);
        query.bindValue(":filename", filename);
        if(query.exec()){
            if(query.next()){
                cnt = query.value(0).toInt();
            }
            if(cnt != 0){
                qDebug("ERROR: the file does already exist!\n");
                message = "ERROR: the file does already exist!\n";
                response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                client->writeData(response);
                //sendMessage(socket, response);
                instance->db.close();
                return;
            }
            else{
                //il file non esiste, quindi posso crearlo
                QSqlQuery query2;
                query2.prepare("SELECT MAX(Id) FROM files"); //l'id del file è un intero crescente: uso MAX perchè l'id deve essere unico per file differenti ma uguale per file uguali, quindi se avessi:
                                                             // 1 prova.txt Mattia
                                                             // 1 prova.txt Ilio   (Ilio e Mattia condividono il file prova.txt)
                                                             // 2 file.txt Ilio
                                                             // 2 file.txt Mattia  --> COUNT ritornerebbe 4 ma io sono arrivato all'id 2, quindi il prossimo dovrebbe essere 3!!
                if(query2.exec()){
                    if(query2.next()){
                        fileId = query2.value(0).toInt() +1;
                    }
                    qDebug()<< "fileId: "<< fileId << "\n";
                    path.append("/").append(username).append("/").append(filename).append(".txt"); //  esempio --> C:/Users/Ilio/Desktop/Progetto_Malnati_git/ilio/prova.txt
                    qDebug()<< "Path: " << path << "\n";

                    QSqlQuery query3;
                    query3.prepare("INSERT INTO files(FileName, Id, userName, Path) VALUES ((:filename), (:fileId), (:username), (:path))");
                    query3.bindValue(":filename", filename);
                    query3.bindValue(":fileId", fileId + 1);
                    query3.bindValue(":username", username);
                    query3.bindValue(":path", path);

                    if(query3.exec()){
                        //QFile file(path);
                        //if(file.open(QIODevice::WriteOnly)){
                        //    QTextStream stream(&file);
                        //    stream << path;
                        //    file.close();
                        //}
                        File *newfile = new File(fileId, path);
                        response = Serialize::fromObjectToArray(Serialize::newFileSerialize(filename, fileId, NEWFILE));
                        //sendSuccess(client);
                        client->writeData(response);

                        instance->files.insert(fileId, newfile);
                        newfile->addUser(client);
                        instance->db.close();
                    }
                    else {
                        qDebug() << "INSERT failed: " << query2.lastError() << "\n";
                        sendError(client);
                        instance->db.close();
                        return;
                    }
                }
                else {
                    qDebug() << "SELECT COUNT(Id) failed: " << query2.lastError() << "\n";
                    sendError(client);
                    instance->db.close();
                    return;
                }
            }
        }
        else {
            qDebug()<< "SELECT COUNT(*) failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
        instance->db.close();
    }
    else {
        qDebug()<<"DB not opened!!\n";
        sendError(client);
        return;
    }
    return;
}

void DBInteraction::sendFileList(ClientManager* client){

    if(!instance->isUserLogged(client)){
        return;
    }

    int userid = client->getId();
    QString username = client->getUsername();

    if(instance->db.open()){
        QSqlQuery query;
        //QJsonArray files; // la lista è vuota?
        QByteArray response;
        QMap<int, QString> files;

        query.prepare("SELECT FileName, Id, Path FROM files WHERE UserName = (:username)");
        query.bindValue(":username", username);

        if(query.exec()){
    
            
            bool atLeastOne = false;

            while(query.next()){
                //per ogni file creo un jsonObject contenente nome del file e id

                QString filename = query.value("FileName").toString();
                int fileId = query.value("Id").toInt();

                //files = Serialize::singleFileSerialize(filename, fileId, files);
                files.insert(fileId, filename);

                atLeastOne = true;
            }


            if (atLeastOne) {

                response = Serialize::fromObjectToArray(Serialize::FileListSerialize(files, SEND_FILES));
                client->writeData(response);
            }
            else {
                qDebug() << "no files";
            }
          //  sendMessage(socket, response);
            instance->db.close();
        }
        else {
            qDebug()<< "SELECT failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug()<<"DB not opened!!\n";
        sendError(client);
        return;
    }
}

void DBInteraction::openFile(int fileId, ClientManager* client, QString URI){

    File *f = nullptr;
    QString message;
    QByteArray response;

    if(!instance->isUserLogged(client)){
        return;
    }

    if (instance->files.contains(fileId)) {
        
        if (instance->files.value(fileId)->getUsers().contains(client)) {
            qDebug() << "file already opened!\n";
            message = "file already opened!";
            response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
            client->writeData(response);
            //sendMessage(socket, response);
            return;
        }
    }

    if(instance->files.contains(fileId)){
        //il file è gia in RAM
        f = DBInteraction::getFile(fileId);
        f->addUser(client);
    }
    else {
        if(URI != nullptr){
            //caso in cui provengo da openSharedFile e quindi ho già la URI(path) del file. Evito cosi un'ulteriore query al server
            f = new File(fileId, URI);
            instance->files.insert(fileId, f);
            f->addUser(client);

        }
        else {
            //cercare il file nel DB
            QString path;
            QSqlQuery query;
            QString username = client->getUsername();
            if(instance->db.open()){
                query.prepare("SELECT path FROM files WHERE username = (:username) AND Id = (:fileid)");
                query.bindValue(":username", username);
                query.bindValue(":fileid", fileId);

                if(query.exec()){
                    if(query.next()){
                        path = QString(query.value("path").toString());
                        f = new File(fileId, path);
                        instance->files.insert(fileId, f);
                        f->addUser(client);
                        qDebug() << "user added!\n";
                        sendSuccess(client);
                        instance->db.close();


                        // che altro fare???
                    }
                    else {
                        qDebug()<<"this file does not exist!\n";
                        message = "this file does not exist!";
                        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                        client->writeData(response);
                        //sendMessage(socket, response);
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
                qDebug()<<"DB not opened!!\n";
                sendError(client);
                return;
            }
        }
    }
}

void DBInteraction::closeFile(int fileId, ClientManager* client){
    //per ogni utente che richiede la chiusura del file verrà fatta una removeUser
    QByteArray response;
    QString message;
    File* f;

    if(!instance->isUserLogged(client)){
        return;
    }

    if(!instance->files.contains(fileId)){ // se il file non si trova nella mappa di file attivi in quel momento vuol dire che o nessuno lo sta utilizzando e quindi è già chiuso oppure che non esiste
        qDebug()<<"this file does not exist or it is not opened!\n";
        message = "this file does not exist or it is not opened!";
        response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
        client->writeData(response);
        //sendMessage(socket, response);
        return;

    }

    if(instance->files.value(fileId)->getUsers().contains(client)){
        //se il file risulta aperto dall'utente allora lo può chiudere
        f = instance->files.value(fileId);
        f->removeUser(client);

        if(!f->thereAreUsers()){
            // in questo momento che il file non è aperto da nessuno, controllo se uno degli utenti ha richiesto di rinominare il file e nel caso lo rinomino
            //se non ho più utenti attivi che lavorano sul file allora lo rimuovo dalla mappa (no dal DB, quindi il file esiste ancora!!)

            if(f->isModifiedName()){
                QString username = client->getUsername();
                QString newName = f->getNewName();
                QString newPath =  QDir::currentPath(); //"C:/Users/Ilio/Desktop/Progetto_Malnati_git/";
                newPath.append(username).append("/").append(newName).append(".txt");

                if(instance->db.open()){
                    QSqlQuery query;
                    query.prepare("UPDATE files SET filename = (:newname), path = (:newpath) WHERE Id = (:fileid) ");
                    query.bindValue("newname", newName);
                    query.bindValue("newpath", newPath);
                    query.bindValue("fileid", fileId);

                    if(query.exec()){
                        sendSuccess(client);

                    }
                    else {
                        qDebug() << "UPDATE failed: " << query.lastError() << "\n";
                        sendError(client);
                        instance->db.close();
                        return;
                    }
                    instance->db.close();
                }
                else {
                    qDebug()<<"DB not opened!!\n";
                    sendError(client);
                    return;
                }
            }
            File* todelte = instance->files.take(fileId);
            instance->files.remove(fileId);
            delete todelte;
            todelte = nullptr;

        }
    }
    return;
}

void DBInteraction::deleteFile(int fileId, ClientManager* client){
    //per ogni utente che richiede la cancellazione verrà cancellata nel DB la riga corrispondente
    QByteArray response;
    QString message;
    QSqlQuery query;



    if(!instance->isUserLogged(client)){
        return;
    }
    QString username = client->getUsername();

    instance->closeFile(fileId, client); // controllo prima se il file è rimasto aperto e se è cosi lo chiudo

    //prima della cancellazione devo salvare di nuovo il file? nel caso di altri utenti attivi sullo stesso

    if(instance->db.open()){
        query.prepare("DELETE FROM files WHERE Id = (:fileid)");// AND UserName = (:username)");
        query.bindValue(":username", username);
        query.bindValue(":fileid", fileId);
        if(query.exec()){
            qDebug() << "file deleted!!\n";
            sendSuccess(client);
            instance->db.close();
        }
        else {
            qDebug() << "DELETE failed: " << query.lastError() << "\n";
            sendError(client);
            instance->db.close();
            return;
        }
    }
    else {
        qDebug()<<"DB not opened!!\n";
        sendError(client);
        return;
    }
    return;
}

void DBInteraction::renameFile(int fileId, QString newName, ClientManager* client){
    // per ogni file bisogna cambiare il nome all'interno del DB e il nome del path (anche nel DB), cosa fatta nella closeFile
    QByteArray response;
    QString message;
    QSqlQuery query;
    File* f;

    if(!instance->isUserLogged(client)){
        return;
    }

    instance->closeFile(fileId, client); // controllo prima se il file è rimasto aperto e se è cosi lo chiudo.

    // nel caso in cui il file sia condiviso tra più utenti, uno di questi vuole cambiare il nome mentre gli altri hanno ancora il file aperto e lo stanno modificando, come faccio?? risposta in closeFile!!
    f = instance->files.value(fileId);
    f->modifyName(newName); //tengo traccia dell'ultimo client che ha richiesto un cambio nome(ogni utente aggiorna la stringa newName contenuta nel file, quindi quella che trovo alla fine sarà l'ultima)

    sendSuccess(client);
}

void DBInteraction::openSharedFile(QString URI, ClientManager* client){

    QByteArray response;
    QString message;

    if(!instance->isUserLogged(client)){
        return;
    }
    //recupero l'ID del file;
    //aggiungo nel DB la riga corrispondente al nuovo utente;
    //chiamo la open
    if(instance->db.open()){
        QSqlQuery query;
        query.prepare("SELECT Id, FileName FROM files WHERE Path = (:path)"); // dovrebbe essere univoca la risposta: i path sono costruiti in modo tale da non poterne avere 2 uguali, quindi la query mi restituisce 1 valore solo
        query.bindValue(":path", URI);

        if(query.exec()){
            int fileId;
            QString filename;
            if(query.next()){
                fileId = query.value("Id").toInt();
                filename = query.value("FileName").toString();
            }
            else {
                qDebug()<<"this file does not exist!\n";
                message = "this file does not exist!";
                response = Serialize::fromObjectToArray(Serialize::responseSerialize(false, message, SERVER_ANSWER));
                client->writeData(response);
                //sendMessage(socket, response);
                instance->db.close();
                return;
            }
            QSqlQuery query2;
            QString username = client->getUsername();
            query2.prepare("INSERT INTO files(FileName, Id, userName, Path) VALUES ((:filename), (:fileId), (:username), (:path))");
            query2.bindValue(":filename", filename);
            query2.bindValue(":fileId", fileId);
            query2.bindValue(":username", username);
            query2.bindValue(":path", URI);
            if(query2.exec()){
                //sendSuccess(client);
                QMap<int, QString> id_file;
                id_file.insert(fileId, filename);
                QByteArray message = Serialize::fromObjectToArray(Serialize::FileListSerialize(id_file, SEND_FILES));
                client->writeData(message);
                //instance->openFile(fileId, client, URI); è meglio aspettare una richiesta specifica dal client per aprire il file
                //perchè se il client fosse lento si perderebbe i messaggi

            }
            else {
                qDebug() << "INSERT failed: " << query.lastError() << "\n";
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
        qDebug()<<"DB not opened!!\n";
        sendError(client);
        return;
    }
}

void DBInteraction::changePassword(QString oldPassword, QString newPassword, ClientManager* client){

    if(!instance->isUserLogged(client)){
        return;
    }

    if(checkPassword(oldPassword, client)){
        if(instance->db.open()){
            QByteArray response;
            QString message;
            QSqlQuery query;
            QByteArray salted_pwd;
            QString hashed_pwd;
            QString salt;
            QString username = client->getUsername();

            salt = instance->generateRandomString(newPassword.size());
            salted_pwd = newPassword.append(salt).toUtf8();
            hashed_pwd = QString(QCryptographicHash::hash(salted_pwd, QCryptographicHash::Sha256));
            qDebug()<<"new password: "<< hashed_pwd<<"\n";

            query.prepare("UPDATE users SET Password = (:newpassword), Salt = (:salt) WHERE Username = (:username)");
            query.bindValue(":newpassword", hashed_pwd);
            query.bindValue(":salt", salt);
            query.bindValue(":username", username);

            if(query.exec()){
                sendSuccess(client);
            }
            else {
                qDebug() << "UPDATE failed: " << query.lastError() << "\n";
                sendError(client);
                instance->db.close();
                return;
            }
        }
        else {
            qDebug()<<"DB not opened!!\n";
            sendError(client);
            return;
        }
    }
}


void DBInteraction::forwardMessage(ClientManager* user, QJsonObject obj, QByteArray data)
{
    //qDebug()<< data;
    QPair<int, Message> fileid_message = Serialize::messageUnserialize(obj);

    //File *f = instance->getFile(fileid_message.first);
    File* f = instance->getFile(fileid_message.first);// debug only
    f->messageHandler(user, fileid_message.second, data);
}

File* DBInteraction::getFile(int fileid){
    return instance->files.value(fileid);
}

bool DBInteraction::isUserLogged(ClientManager* client){

    if(!activeusers.contains(client)){
        qDebug()<<"user not authorized!\n";
        //anche se non è attivo, l'utente ha comunque un socket
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

//generazione casuale di un colore ed inserimento nella lista così che non venga ripetuto
QColor DBInteraction::generateRandomColor(int userId){
    QColor newColor;
    do{
       newColor = QColor::fromRgb(QRandomGenerator::global()->generate());
    }
    while(colorPresent(newColor));
    m_colorPerUser.insert(userId, newColor);
    return newColor;
}

bool DBInteraction::colorPresent(QColor color){
    for(auto it = m_colorPerUser.begin(); it != m_colorPerUser.end(); it++){
        if(it.value() == color) return true;
    }
    return false;
}
