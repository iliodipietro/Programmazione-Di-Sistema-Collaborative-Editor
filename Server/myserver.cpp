#include "myserver.h"

MyServer::MyServer(QObject *parent) : QObject (parent), _server(new QTcpServer(this)), m_lastId(0)
{
    //supporto al file system da implementare
    db->startDBConnection();
    connect(_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    //listen(QHostAddress("192.168.0.6"), 44322);
    listen(QHostAddress::Any, 44322); //la liste va chiamata altrimenti il server non sa che indirizzo e porta ascoltare
    //connect(this, SIGNAL(bufferReady(QTcpSocket*, QByteArray)), SLOT(MessageHandler(QTcpSocket*,QByteArray)));

    // solo per fare prove
    db->funzionedaeliminare();

    QString path(QDir::currentPath() + "/log.txt");
     m_logFile = new QFile(path);
    if(m_logFile->open(QIODevice::WriteOnly)){
        m_logFileStream = new QTextStream(m_logFile);
        *m_logFileStream << "prova";
        m_logFileStream->flush();
    }

}

bool MyServer:: listen(QHostAddress addr, quint16 port){

    if(!_server->listen(addr, port)){
        qCritical("Cannot connect server: %s", qPrintable(_server->errorString()));
        return false;
    }
    qDebug("Server is listening on: %s:%d", qPrintable(addr.toString()), port);
    return true;
}

//ad ogni nuova connessione il server usa l'istanza del socket per creare una classe ClientManager si occuper�  di leggere e scrivere i messaggi
void MyServer::onNewConnection(){
    while(_server->hasPendingConnections()){
        QTcpSocket *newConnection = _server->nextPendingConnection();
        if(newConnection == nullptr) return;

        qDebug("New connection from %s:%d.", qPrintable(newConnection->peerAddress().toString()), newConnection->peerPort());
        ClientManager* client = new ClientManager(newConnection, this);
        m_connectedClients.push_back(client); //ogni client viene inserito in una lista per tenere traccia di quelli connessi
        //connect(newConnection, SIGNAL(readyRead()), this, SLOT(readFromSocket())); //This signal is emitted once every time new data is available for reading from the device's current read channel
        //connect(newConnection, SIGNAL(disconnected()), this, SLOT(onDisconnect())); //This signal is emitted when the socket has been disconnected.
        connect(client, &ClientManager::messageReceived, this, &MyServer::MessageHandler);
        connect(client, &ClientManager::disconnected, this, &MyServer::onDisconnect);
    }
}

/*void MyServer::readFromSocket(){
    //usando TCP, abbiamo un FLUSSO CONTINUO di dati e per questo motivo è necessario un meccanismo per capire dove inizia e dove finisce un singolo dato inviato dal client.
    //In questa soluzione abbiamo scelto di inviare per prima cosa la dimensione "dim" del dato da leggere, per poi leggere tutti i restandi "dim" byte che rappresentano il dato completo

    QTcpSocket *sender = static_cast<QTcpSocket*>(QObject::sender()); //sender() returns a pointer to the object that sent the signal, if called in a slot activated by a signal; otherwise it returns nullptr.
    QByteArray *buffer;
    QByteArray dataToHandle;
    //quint64 dim = 0;
    quint32 dim = 0;
    auto buffer_socket = socket_buffer.find(sender);

    if(buffer_socket == socket_buffer.end()){
        socket_buffer.insert(sender, {buffer, dim});
    }
    else {
        buffer = buffer_socket.value().first;
        dim    = buffer_socket.value().second;
    }

    while(sender->bytesAvailable() || buffer->size() != 0){

        buffer->append(sender->readAll());
        qDebug()<<"data read: "<< buffer<<"\n";

        while((dim == 0 && buffer->size() >= 8) || (dim > 0 && static_cast<quint64>(buffer->size()) >= dim)){

            if(dim == 0 && buffer->size() >= 8){ // è stata ricevuta la dimensione del buffer (primo parametro)
                //dim = buffer->mid(0,8).toULongLong(); //prendo i primi 8 byte che rappresentano la dimensione

                dim = atoi(buffer->mid(0,8).data());
                qDebug()<< "size of data: "<< dim<< "\n";
                buffer->remove(0,8); //rimuovo dal buffer i primi 8 byte, cosi da poter leggere i veri e propri dati
            }
            if(dim > 0 && static_cast<quint64>(buffer->size()) >= dim){ // ho precedentemente ricevuto la dimensione del dato, quindi adesso lo leggo tutto ed emetto il segnale per

                /*if(dim <= std::numeric_limits<quint32>::max()){ //la dimensione del dato da leggere è più piccola di un int: posso usare tranquillamente la funzione mid
                    dataToHandle = buffer->mid(0, static_cast<quint32>(dim));
                    buffer->remove(0, static_cast<quint32>(dim));
                    dim = 0;
                }
                else{ // la dimensione è più grande di un intero (32 bit)
                    while(dim != 0){
                        if(dim >= std::numeric_limits<quint32>::max()){
                            dataToHandle = buffer->mid(0, std::numeric_limits<quint32>::max());
                            buffer->remove(0, std::numeric_limits<quint32>::max());
                            dim -= std::numeric_limits<quint32>::max();
                        }
                        else{
                            //ora la dimensione (dim) è sicuramente su 32 bit, allora posso fare tranquillamente il cast senza il rischio di perdere informazioni
                            dataToHandle = buffer->mid(0, static_cast<quint32>((dim)));
                            buffer->remove(0, static_cast<quint32>((dim)));
                            dim = 0;
                        }
                    }
                }*/
                /*dataToHandle = buffer->mid(0, dim);
                buffer->remove(0, dim);
                dim = 0;

                emit dataReady(sender, dataToHandle);
            }
        }
    }
}*/

void MyServer::MessageHandler(ClientManager *client, QByteArray socketData){

    /*

#define LOGIN 1
#define LOGOUT 2
#define REGISTER 3
#define FILENAME 4
#define MESSAGE 5
#define TEXT 6
#define IMAGE 7
#define NEWFILE 8
#define OPEN 9
#define CLOSE 10
#define CURSOR 11
#define SERVER_ANSWER 12
#define DELETE 13
#define RENAME 14
#define SHARE 15
#define SEND_FILES 16
#define CHANGE_PASSWORD 17
#define CHANGE_USERNAME 18
#define CHANGE_NICK 19
#define CHANGE_PROPIC 20

    */

    QJsonObject ObjData = Serialize::fromArrayToObject(socketData);
    QStringList list;
    int fileId;
    QString username, filename, newName, URI;
    QPair<int, QString> rename;
    //QPair<int, Message> fileid_message;

    int type = Serialize::actionType(ObjData);
    qDebug()<<"request: "<<type<<"\n";


    switch (type) {
    case (LOGIN):
        qDebug("LOGIN request\n");

         list = Serialize::userUnserialize(ObjData);
         db->login(list.at(0), list.at(1), client);

         list.clear();

        break;
    case (LOGOUT):
        qDebug("LOGOUT request\n");
        //il messaggio di logout contiene solo il type LOGOUT
        db->logout(client);

        break;
    case (REGISTER): {
        qDebug("REGISTER request\n");
        list = Serialize::userUnserialize(ObjData);
        db->registration(list.at(0), list.at(1), list.at(2), list.at(3), client);

        list.clear();

        break;
    }
    case (FILENAME):
        qDebug("FILENAME request\n");

        break;
    case (MESSAGE):
        qDebug("MESSAGE request");
        db->forwardMessage(client,ObjData,socketData);

        //fileid_message = Serialize::messageUnserialize(ObjData);

        //f = db->getFile(fileid_message.first);
        //f->messageHandler(socket, fileid_message.second, socketData);

        break;
    case (TEXT):
        qDebug("TEXT request\n");

        break;
    case (IMAGE):
        qDebug("IMAGE request\n");

        break;
    case (NEWFILE):
        qDebug("NEWFILE request\n");
        filename = Serialize::newFileUnserialize(ObjData);

        db->createFile(filename, client);

        break;
    case (OPEN):{
        qDebug("OPEN request\n");
        fileId = Serialize::openCloseDeleteFileUnserialize(ObjData);

        db->openFile(fileId, client);

        break;
    }
    case (CLOSE):
        qDebug("CLOSE request\n");
        fileId = Serialize::openCloseDeleteFileUnserialize(ObjData);

        db->closeFile(fileId, client);

        break;
    case (CURSOR):{
        qDebug("CURSOR request");
        int fileId = ObjData.value("fileId").toInt();
        File* f = db->getFile(fileId);
        f->updateCursorPosition(client, socketData);

        break;
    }
    case (SERVER_ANSWER):
        qDebug("SERVER_ANSWER request\n");

        break;
    case (DELETE):
        qDebug("DELETE request\n");
        fileId = Serialize::openCloseDeleteFileUnserialize(ObjData);

        db->deleteFile(fileId, client);

        break;
    case (RENAME):
        qDebug("RENAME request\n");
        rename = Serialize::renameFileUnserialize(ObjData);

        db->renameFile(rename.first, rename.second, client);

        break;
    case (SHARE):
        qDebug("SHARE request\n");
        URI = Serialize::openSharedFileUnserialize(ObjData);

        db->openSharedFile(URI, client);

        break;
    case (SEND_FILES):
        qDebug("SEND_FILES request\n");

        db->sendFileList(client);

        break;
    case (CHANGE_PASSWORD):
        qDebug("CHANGE_PASSWORD request\n");
        list = Serialize::changePasswordUnserialize(ObjData);

        db->changePassword(list.at(0), list.at(1), client);

        list.clear();

        break;
    case (CHANGE_USERNAME):
        qDebug("CHANGE_USERNAME request\n");

        break;
    case (CHANGE_NICK):
        qDebug("CHANGE_NICK request\n");

        break;
    case (CHANGE_PROPIC):
        qDebug("CHANGE_PROPIC request\n");

        break;

    default:
        QString str = QString::fromUtf8(socketData);
        qDebug()<<str<<"\n";
        break;
    }


}

//quando un client si disconnette viene rimosso dalla lista di quelli connessi
void MyServer::onDisconnect(){
    ClientManager* client = static_cast<ClientManager*>(sender());
    db->logout(client);
    auto it = m_connectedClients.begin();
    for(it; it != m_connectedClients.end(); it++){
        if((*it) == client){
            m_connectedClients.erase(it);
            break;
        }
    }
    client->deleteLater();
}

void MyServer::handleMessage(int fileID, Message m)
{
    int senderId = m.getSenderId();

    this->fileId_CRDT.at(fileID)->process(m);

    //@TODO fare for per mandare a tutti gli utenti che lavorano su quel file tranne a chi ha inviato
}

//std::vector<Message> MyServer::readFileFromDisk(std::string path, int fileID)
//{
//    auto it = this->fileId_CRDT.find(fileID);
//
//    if (this->addFile(fileID,path)) {//true se è andato a buon fine
//
//
//        //@TODO--> vedere se è la prima volta che il file viene creato o meno--> se è nuovo non faccio read
//        auto vett = this->fileId_CRDT.at(fileID)->readFromFile();
//
//        sendNewFile(vett, fileID);
//    }
//    else {
//        return std::vector<Message>();
//    }
//
//    return std::vector<Message>();
//}

void MyServer::sendNewFile(std::vector<Message> messages, int fileId)
{
    for (auto m : messages) {
        //@TODO altro for per madare a tutti quelli chevogliono lavorare sul file
    }
}

//bool MyServer::addFile(int fileID, std::string path)
//{
//    auto it = this->fileId_CRDT.find(fileID);
//
//    if (it != fileId_CRDT.end())
//        return false;//gia presente qull'ID
//
//    CRDT* file = new CRDT(fileID,path);
//
//    this->fileId_CRDT.insert(std::pair<int, CRDT*>(fileID, file));//non presente aggiungo
//
//    return true;
//}
//
//void MyServer::removeFile(int fileID)
//{
//    auto it = this->fileId_CRDT.find(fileID);
//
//    if (it != fileId_CRDT.end()) {
//
//        this->fileId_CRDT.erase(it);
//    }
//}

MyServer::~MyServer()
{
    m_logFile->close();
}
