#include "myserver.h"




MyServer::MyServer(QObject *parent) : QObject (parent), _server(new QTcpServer(this)){
    //supporto al file system da implementare
    db->startDBConnection();
    connect(_server, SIGNAL(newConnection()), SLOT(onNewConnection()));
    connect(this, SIGNAL(bufferReady(QTcpSocket*, QByteArray)), SLOT(MessageHandler(QTcpSocket*,QByteArray)));

    //this->addFile(0, "C:/Users/Mattia Proietto/Desktop/prova_save_Copia.txt");
}

bool MyServer:: listen(QHostAddress &addr, quint16 port){

    if(!_server->listen(addr, port)){
        qCritical("Cannot connect server: %s", qPrintable(_server->errorString()));
        return false;
    }
    qDebug("Server is listening on: %s:%d", qPrintable(addr.toString()), port);
    return true;
}

void MyServer::onNewConnection(){
    QTcpSocket *newConnection = _server->nextPendingConnection();
    if(newConnection == nullptr) return;

    qDebug("New connection from %s:%d.", qPrintable(newConnection->peerAddress().toString()), newConnection->peerPort());

    connect(newConnection, SIGNAL(readyRead()), this, SLOT(readFromSocket())); //This signal is emitted once every time new data is available for reading from the device's current read channel
    connect(newConnection, SIGNAL(disconnected()), this, SLOT(onDisconnect())); //This signal is emitted when the socket has been disconnected.
}

void MyServer::readFromSocket(){
    QTcpSocket *sender = static_cast<QTcpSocket*>(QObject::sender()); //sender() returns a pointer to the object that sent the signal, if called in a slot activated by a signal; otherwise it returns nullptr.
    qint64 num_byte = sender->bytesAvailable();
    QByteArray buffer;

    auto buffer_socket = socket_buffer.find(sender);

    if(buffer_socket == socket_buffer.end()){
        socket_buffer.insert(sender, buffer);
    }
    else {
        buffer = buffer_socket.value();
    }



    if(num_byte > 0){

    }
}

void MyServer::MessageHandler(QTcpSocket *socket, QByteArray socketData){

    /*
    LOGIN 1
    REGISTER 2
    FILENAME 3
    MESSAGE 4
    TEXT 5
    IMAGE 6
    OPEN 7
    CLOSE 8
    CURSOR 9
    SERVER_ANSWER 10

    */
    QJsonObject ObjData = Serialize::fromArrayToObject(socketData);
    QStringList list;

    int type = Serialize::actionType(ObjData);

    switch (type) {
    case (LOGIN):
        qDebug("LOGIN request");

         list = Serialize::userUnserialize(ObjData);
         db->login(list.at(0), list.at(1), socket);

        break;
    case (REGISTER):
        qDebug("REGISTER request");

        list = Serialize::userUnserialize(ObjData);
        db->registration(list.at(0), list.at(1), socket);

        break;
    case (FILENAME):
        qDebug("FILENAME request");

        break;
    case (MESSAGE):
        qDebug("MESSAGE request");

        break;
    case (TEXT):
        qDebug("TEXT request");

        break;
    case (IMAGE):
        qDebug("IMAGE request");

        break;
    case (OPEN):
        qDebug("OPEN request");

        break;
    case (CLOSE):
        qDebug("CLOSE request");

        break;
    case (CURSOR):
        qDebug("CURSOR request");

        break;
    case (SERVER_ANSWER):
        qDebug("SERVER_ANSWER request");

        break;



    }


}

void MyServer::onDisconnect(){

}



void MyServer::handleMessage(int fileID, Message m)
{
    int senderId = m.getSenderId();

    this->fileId_CRDT.at(fileID)->process(m);

    //@TODO fare for per mandare a tutti gli utenti che lavorano su quel file tranne a chi ha inviato
}

std::vector<Message> MyServer::readFileFromDisk(std::string path, int fileID)
{
    auto it = this->fileId_CRDT.find(fileID);

    if (this->addFile(fileID,path)) {//true se è andato a buon fine
        

        //@TODO--> vedere se è la prima volta che il file viene creato o meno--> se è nuovo non faccio read
        auto vett = this->fileId_CRDT.at(fileID)->readFromFile();

        sendNewFile(vett, fileID);
    }
    else {
        return std::vector<Message>();
    }
    
    return std::vector<Message>();
}

void MyServer::sendNewFile(std::vector<Message> messages, int fileId)
{
    for (auto m : messages) {
        //@TODO altro for per madare a tutti quelli chevogliono lavorare sul file 
    }
}

bool MyServer::addFile(int fileID, std::string path)
{
    auto it = this->fileId_CRDT.find(fileID);

    if (it != fileId_CRDT.end())
        return false;//gia presente qull'ID

    CRDT* file = new CRDT(fileID,path);

    this->fileId_CRDT.insert(std::pair<int, CRDT*>(fileID, file));//non presente aggiungo

    return true;
}

void MyServer::removeFile(int fileID)
{
    auto it = this->fileId_CRDT.find(fileID);

    if (it != fileId_CRDT.end()) {

        this->fileId_CRDT.erase(it);
    }
}

MyServer::~MyServer()
{

}
