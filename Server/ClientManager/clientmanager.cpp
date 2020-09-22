#include "clientmanager.h"
#include <QDataStream>

ClientManager::ClientManager(QTcpSocket* socket, QObject *parent) : QObject(parent),
m_clientSocket(socket), m_socketBuffer(new QByteArray())
{
    m_clientSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    //connect(m_clientSocket.get(), SIGNAL(connected()), this, SLOT(connected()));
    connect(m_clientSocket.get(), SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(m_clientSocket.get(), SIGNAL(readyRead()), this, SLOT(readyRead()));
    //connect(m_clientSocket.get(), SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
}

void ClientManager::readyRead(){
    //usando TCP, abbiamo un FLUSSO CONTINUO di dati e per questo motivo è necessario un meccanismo per capire dove inizia e dove finisce un singolo dato inviato dal client.
    //In questa soluzione abbiamo scelto di inviare per prima cosa la dimensione "dim" del dato da leggere, per poi leggere tutti i restandi "dim" byte che rappresentano il dato completo
    QByteArray dataToHandle;
    qint64 dim = 0;
    while(m_clientSocket->bytesAvailable() || m_socketBuffer->size() != 0){
        m_socketBuffer->append(m_clientSocket->readAll());
        while((dim == 0 && m_socketBuffer->size() >= 8) || (dim > 0 && m_socketBuffer->size() >= dim)){
            if(dim == 0 && m_socketBuffer->size() >= 8){ // è stata ricevuta la dimensione del m_socketBuffer (primo parametro)
                //dim = m_socketBuffer->mid(0,8).toULongLong(); //prendo i primi 8 byte che rappresentano la dimensione
                dim = arrayToInt(m_socketBuffer->mid(0,8));
                m_socketBuffer->remove(0,8); //rimuovo dal m_socketBuffer i primi 8 byte, cosi da poter leggere i veri e propri dati
            }
            if(dim > 0 && m_socketBuffer->size() >= dim){ // ho precedentemente ricevuto la dimensione del dato, quindi adesso lo leggo tutto ed emetto il segnale per
                dataToHandle = m_socketBuffer->mid(0, dim);
                m_socketBuffer->remove(0, dim);
                dim = 0;
                emit messageReceived(this, dataToHandle); //il pacchetto letto viene mandato al message handler tramite un segnale
            }
        }
    }
}

//sarebbe meglio usare questa funzione per scrivere i messaggi sul socket invece che creare tante funzioni sparse
//in giro per tutto il progetto
bool ClientManager::writeData(QByteArray& data) {
    if (m_clientSocket->state() == QAbstractSocket::ConnectedState)
    {
        m_clientSocket->write(intToArray(data.size()).append(data));
        return m_clientSocket->waitForBytesWritten();
    }
    else {
        return false;
    }
}

QTcpSocket* ClientManager::getSocket()
{
    return this->m_clientSocket.get();
}

void ClientManager::setId(int id)
{
    this->m_id = id;
}

QString ClientManager::getUsername(){
    return this->m_username;
}

QString ClientManager::getNickname(){
    return this->m_nickname;

}

void ClientManager::setNickname(QString nickname){
    this->m_nickname = nickname;

}
int ClientManager::getId(){
    return this->m_id;
}

//la disconnessione viene segnalata al server
void ClientManager::onDisconnect(){
    emit disconnected();
}
//L'username del client viene aggiunto solo una volta che il login è stato effettuato con successo
void ClientManager::setUsername(QString username){
    m_username = username;
}
//Il colore del client viene aggiunto solo una volta che il login è stato effettuato con successo
void ClientManager::setColor(QColor color){
    m_color = color;
}


//conversione da QByteArray a qint64
qint64 ClientManager::arrayToInt(QByteArray source){
    qint64 temp;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> temp;
    return temp;
}

//conversione da qint64 a QByteArray
QByteArray ClientManager::intToArray(qint64 source) {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

ClientManager::~ClientManager(){

}
