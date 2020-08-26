#include "myserver.h"




MyServer::MyServer(QObject *parent) : QObject (parent), _server(new QTcpServer(this)){
    //supporto al file system da implementare
    db = DBInteraction::startDBConnection();
    if(db == nullptr){
        return; // giusto? ----------------------->(periodicamente riprovare la connessione al DB!!)
    }
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
                dataToHandle = buffer->mid(0, dim);
                buffer->remove(0, dim);
                dim = 0;

                emit dataReady(sender, dataToHandle);
            }
        }
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
    int fileId;
    QString username, filename;

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
    case (NEWFILE):
        qDebug("NEWFILE request");
        filename = Serialize::newFileUnserialize(ObjData).first;
        username = Serialize::newFileUnserialize(ObjData).second;

        db->createFile(filename, username, socket);

        break;
    case (OPEN):
        qDebug("OPEN request");
        fileId = Serialize::openCloseFileUnserialize(ObjData).first;
        username = Serialize::openCloseFileUnserialize(ObjData).second;
        db->openFile(fileId, username, socket);

        break;
    case (CLOSE):
        qDebug("CLOSE request");
        fileId = Serialize::openCloseFileUnserialize(ObjData).first;
        username = Serialize::openCloseFileUnserialize(ObjData).second;
        db->closeFile(fileId, username, socket);

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
