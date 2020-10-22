#include "file.h"

File::File():handler(nullptr),id(0),path("")
{

}

File::~File()
{
	this->handler->saveOnFile();//prima di eliminare salvo a prescinedere
	delete this->handler;
	this->handler = nullptr;
}

File::File(int fileId, QString path): id(fileId),path(path){
	//creo il crdt e se il file puntato dal path non è vuoto questo viene caricato automaticamente nel CRDT
    this->handler = new CRDT(fileId, this->path);
    this->modifiedName = false;
    this->newName = nullptr;
}

void File::messageHandler(ClientManager* sender, Message m, QByteArray bytes)
{
    std::vector<int> pos;
    bool afterInsert = true;
    if (m.getAction() != CURSOR_S) {
		this->handler->process(m);//i cursori non sono slavati nel CRDT

	/*faccio partire il timer della durata definita da TIMEOUT in CRDT.h se alla scadenza del timer non ho ancora ricevuto alcun nuovo messaggio
	  salvo il credt su file altrimenti il timer viene fatto ripartire e si ricomincia dalla condizione precedente.
	*/
		this->handler->getTimer()->start(TIMEOUT);

		//this->handler->printText();

        pos = m.getSymbol().getPos();

        if(m.getAction() != INSERT_SYMBOL)
            afterInsert = false;

        //if(m.getAction() == DELETE_SYMBOL) pos--;
    }
    else{
        pos = m.getCursorPosition();
        afterInsert = false;
    }

    CursorPosition CP(pos, afterInsert);

    if(m_usersCursorPosition.find(sender) != m_usersCursorPosition.end()){
        m_usersCursorPosition[sender] = CP;
    }
    else{
        m_usersCursorPosition.insert(sender, CP);
    }

	QList<int> keys = this->users.keys();

	for (auto u : keys) {
		if (sender->getId() != u) {
			//madare messaggi su tutti i socket tranne che al sender
			//mando direttamente il messagio sotto forma di bytearray
			this->users.value(u)->writeData(bytes);

		}
	}
}

void File::sendNewFile(ClientManager* socket)
{
	if (!this->handler->isEmpty()) {
		//se e esolo se non è vuoto--> nuovo file o senza caratteri
		//prendo tutto il testo come vettore di messagi, e poi un messagio per volta lo serializzo, trasformo in array e lo invio
		std::vector<Message> msgs = this->handler->getMessageArray();

		for (auto m : msgs) {
            QByteArray bytes = Serialize::fromObjectToArray(Serialize::messageSerialize(this->id, m, MESSAGE));
			socket->writeData(bytes);
		}

        for(auto it = m_usersCursorPosition.begin(); it!=m_usersCursorPosition.end(); it++){
            std::vector<int> pos;
            if(it.value().afterInsert)
                pos = this->handler->getNextCursorPosition(it.value().pos);
            Message cursorPosition(pos, CURSOR_S, it.key()->getId());
            QByteArray bytes = Serialize::fromObjectToArray(Serialize::messageSerialize(this->id, cursorPosition, MESSAGE));
            socket->writeData(bytes);
        }
	}
}

bool File::isModifiedName(){
    return this->modifiedName;
}

QString File::getNewName(){
    return this->newName;
}

QString File::getPath(){
    return this->path;
}

void File::modifyName(QString newName){
    this->modifiedName = true;
    this->newName = newName;
}

void File::addUser(ClientManager* user)
{
	//quando aggiungo un nuovo utente gli mando l'intero testo
	if (!this->users.contains(user->getId())) {

        this->users.insert(user->getId(), user);
		//this->users.append(user);
        for(auto it = users.begin(); it != users.end(); it++){
            if((*it)->getId() != user->getId()){
                QByteArray message = Serialize::fromObjectToArray(Serialize::addEditingUserSerialize(user->getId(), user->getUsername(), user->getColor(), this->id, NEWEDITINGUSER));
                (*it)->writeData(message);
                message = Serialize::fromObjectToArray(Serialize::addEditingUserSerialize((*it)->getId(), (*it)->getUsername(), (*it)->getColor(), this->id, NEWEDITINGUSER));
                user->writeData(message);
            }
        }
        this->sendNewFile(user);

	}

}

void File::removeUser(ClientManager* user)
{
	//rimuovo un utente che non lavora piu sul file
	this->users.remove(user->getId());
    m_usersCursorPosition.remove(user);
    for(auto it = users.begin(); it != users.end(); it++){
        if((*it)->getId() != user->getId()){
            QByteArray message = Serialize::fromObjectToArray(Serialize::removeEditingUserSerialize(user->getId(), this->id, REMOVEEDITINGUSER));
            (*it)->writeData(message);
        }
    }
}

QList<ClientManager*> File::getUsers()
{
	//ritorno la lista di utenti attivi visti come socket
	return this->users.values();
	
}

QList<ClientManager*> File::getRUsers()
{
    return this->r_users;
}

void File::setSharedFile(){
    this->shared = true;
}

bool File::is_file_shared()
{
    return this->shared;
}

void File::addRUser(ClientManager* client){
    this->r_users.append(client);
}

bool File::thereAreUsers()
{
	//mi dice se qualcuno sta ancora lavorando o meno sul file
	return this->users.size() > 0 ;
}
