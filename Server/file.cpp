#include "file.h"

File::File():handler(nullptr),id(0),path("")
{

}

File::File(int fileId, QString path): id(fileId),path(path){
	//creo il crdt e se il file puntato dal path non è vuoto questo viene caricato automaticamente nel CRDT
	this->handler = new CRDT(fileId, this->path);
}

void File::messageHandler(ClientManager* sender, Message m, QByteArray bytes)
{
	if (m.getAction() != CURSOR_S) {
		this->handler->process(m);//i cursori non sono slavati nel CRDT
		this->handler->getTimer()->start(TIMEOUT);
	}
	for (auto u : this->users) {
		if (sender != u) {
			//madare messaggi su tutti i socket tranne che al sender
			//mando direttamente il messagio sotto forma di bytearray
			u->writeData(bytes);

		}
	}
	/*faccio partire il timer della durata definita da TIMEOUT in CRDT.h se alla scadenza del timer non ho ancora ricevuto alcun nuovo messaggio
	  salvo il credt su file altrimenti il timer viene fatto ripartire e si ricomincia dalla condizione precedente.
	*/
	
}

void File::sendNewFile(ClientManager* socket)
{
	if (!this->handler->isEmpty()) {
		//se e esolo se non è vuoto--> nuovo file o senza caratteri
		//prendo tutto il testo come vettore di messagi, e poi un messagio per volta lo serializzo, trasformo in array e lo invio
		std::vector<Message> msgs = this->handler->getMessageArray();

		for (auto m : msgs) {
			QByteArray bytes = Serialize::fromObjectToArray(Serialize::messageSerialize(this->id, m, INSERT_SYMBOL));
			socket->writeData(bytes);
		}
	}
}

void File::addUser(ClientManager* user)
{
	//quando aggiungo un nuovo utente gli mando l'intero testo
	this->users.append(user);
	this->sendNewFile(user);

}

void File::removeUser(ClientManager* user)
{
	//rimuovo un utente che non lavora piu sul file
	this->users.removeOne(user);
}

QVector<ClientManager*> File::getUsers()
{
	//ritorno la lista di utenti attivi visti come socket
	return this->users;
}

bool File::thereAreUsers()
{
	//mi dice se qualcuno sta ancora lavorando o meno sul file
	return this->users.size() > 0 ;
}

