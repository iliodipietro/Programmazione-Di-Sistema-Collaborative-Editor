#include "myserver.h"




MyServer::MyServer(QObject *parent) : QObject (parent), _server(new QTcpServer(this)){
    //supporto al file system da implementare
    db->startDBConnection();
    connect(_server, SIGNAL(newConnection()), SLOT(onNewConnection()));
    connect(this, SIGNAL(bufferReady(QTcpSocket*, QByteArray)), SLOT(MessageHandler(QTcpSocket*,QByteArray)));


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


MyServer::~MyServer()
{

}
