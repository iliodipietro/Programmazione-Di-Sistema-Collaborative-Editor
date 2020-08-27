#include "file.h"

File::File():handler(nullptr),id(0),path("")
{

}

File::File(int fileId, QString path): id(fileId),path(path){
	//creo il crdt e se il file puntato dal path non è vuoto questo viene caricato automaticamente nel CRDT
	this->handler = new CRDT(fileId, this->path);
}

void File::messageHandler(QTcpSocket* sender, Message m, QByteArray bytes)
{
	if (m.getAction() != CURSOR_S) {
		this->handler->process(m);//i cursori non sono slavati nel CRDT
		this->handler->getTimer()->start(TIMEOUT);
	}
	for (auto u : this->users) {
		if (sender != u) {
			//madare messaggi su tutti i socket tranne che al sender
			//mando direttamente il messagio sotto forma di bytearray
			this->writeData(u, bytes);

		}
	}
	/*faccio partire il timer della durata definita da TIMEOUT in CRDT.h se alla scadenza del timer non ho ancora ricevuto alcun nuovo messaggio
	  salvo il credt su file altrimenti il timer viene fatto ripartire e si ricomincia dalla condizione precedente.
	*/
	
}

void File::sendNewFile(QTcpSocket* socket)
{
	if (!this->handler->isEmpty()) {
		//se e esolo se non è vuoto--> nuovo file o senza caratteri
		//prendo tutto il testo come vettore di messagi, e poi un messagio per volta lo serializzo, trasformo in array e lo invio
		std::vector<Message> msgs = this->handler->getMessageArray();

		for (auto m : msgs) {
			QByteArray bytes = Serialize::fromObjectToArray(Serialize::messageSerialize(this->id, m, INSERT_SYMBOL));
			this->writeData(socket, bytes);
		}
	}
}

void File::addUser(QTcpSocket* user)
{
	//quando aggiungo un nuovo utente gli mando l'intero testo
	this->users.append(user);
	this->sendNewFile(user);

}

void File::removeUser(QTcpSocket* user)
{
	//rimuovo un utente che non lavora piu sul file
	this->users.removeOne(user);
}

QVector<QTcpSocket*> File::getUsers()
{
	//ritorno la lista di utenti attivi visti come socket
	return this->users;
}

bool File::thereAreUsers()
{
	//mi dice se qualcuno sta ancora lavorando o meno sul file
	return this->users.size() > 0 ;
}



void File::writeData(QTcpSocket* socket, QByteArray bytes)
{
	if (socket->state() == QAbstractSocket::ConnectedState) {
		qint32 msg_size = bytes.size();
		QByteArray toSend;
		socket->write(toSend.number(msg_size), sizeof(quint64)); //la funzione number converte il numero che rappresenta la dimensione del dato da inviare (msg_size) in stringa (es. 100 --> "100").
																  //Siccome una stringa occupa di piu del relativo numero ("100" occupa 8*3 bit mentre 100 ne occupa solo 8), tale stringa viene mandata sul socket
																  // su 64 bit invece di 32 che rappresenta la massima dimensione possibile di un dato
		socket->waitForBytesWritten();
		qint32 byteWritten = 0;
		while (byteWritten < msg_size) {
			byteWritten += socket->write(bytes);
			socket->waitForBytesWritten();
		}
	}
}
