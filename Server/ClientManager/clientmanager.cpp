#include "clientmanager.h"

ClientManager::ClientManager(int id, QTcpSocket* socket, QObject *parent) : QObject(parent),
    m_id(id), m_clientSocket(socket)
{
    m_clientSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    connect(m_clientSocket.get(), SIGNAL(connected()), this, SLOT(connected()));
    connect(m_clientSocket.get(), SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_clientSocket.get(), SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(m_clientSocket.get(), SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
}

void ClientManager::readyRead(){
    //usando TCP, abbiamo un FLUSSO CONTINUO di dati e per questo motivo è necessario un meccanismo per capire dove inizia e dove finisce un singolo dato inviato dal client.
    //In questa soluzione abbiamo scelto di inviare per prima cosa la dimensione "dim" del dato da leggere, per poi leggere tutti i restandi "dim" byte che rappresentano il dato completo
    QByteArray dataToHandle;
    qint64 dim = 0;

    while(m_clientSocket->bytesAvailable() || m_socketBuffer->size() != 0){

        m_socketBuffer->append(m_clientSocket->readAll());
        qDebug()<<"data read: "<< m_socketBuffer<<"\n";

        while((dim == 0 && m_socketBuffer->size() >= 8) || (dim > 0 && m_socketBuffer->size() >= dim)){

            if(dim == 0 && m_socketBuffer->size() >= 8){ // è stata ricevuta la dimensione del m_socketBuffer (primo parametro)
                //dim = m_socketBuffer->mid(0,8).toULongLong(); //prendo i primi 8 byte che rappresentano la dimensione

                dim = atoi(m_socketBuffer->mid(0,8).data());
                qDebug()<< "size of data: "<< dim<< "\n";
                m_socketBuffer->remove(0,8); //rimuovo dal m_socketBuffer i primi 8 byte, cosi da poter leggere i veri e propri dati
            }
            if(dim > 0 && m_socketBuffer->size() >= dim){ // ho precedentemente ricevuto la dimensione del dato, quindi adesso lo leggo tutto ed emetto il segnale per
                dataToHandle = m_socketBuffer->mid(0, static_cast<int>(dim));
                m_socketBuffer->remove(0, static_cast<int>(dim));
                dim = 0;

                emit messageReceived(m_clientSocket.get(), dataToHandle);
            }
        }
    }
}

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

ClientManager::~ClientManager(){

}
