#include "CRDT.h"
#include <QTextStream>
#include <Qstring>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>

/*

id dato dal server per identificare il singolo client usato per creare l'id univoco globale del carattere
il crdt dovrebbe girare anche sul server --> server ha id == 0 e non crea mai caratteri

*/

CRDT::CRDT(int id) :_siteId(id), _counter(0)
{
	this->_symbols.reserve(200000);
}


CRDT::~CRDT()
{
}
//inserimento e cancellazione nelle strutture dati locali sono le remote insert/delete di conclave ossia de messaggi che mi arrivano
__int64 CRDT::insert_symbol(Symbol symbol)
{
	__int64 index = -1;


	if (this->_symbols.empty() || symbol.getPos() > this->_symbols.back().getPos()) {
		//inserisco in coda, lo faccio come prima operazione in modo da ottimizzare l'inserimento
		//quando carico dal server
		_symbols.push_back(symbol);
		index = _symbols.size() - 1;
	}
	else {

		//trovo l'iteratore che punta alla posizione in cui inserire basandomi sulle pos frazionarie
		auto it = std::find_if(this->_symbols.begin(), this->_symbols.end(), [symbol](Symbol s) {

			if (s.getPos() > symbol.getPos())
				return true;
			else if (s.getPos() == symbol.getPos()) {

				if (symbol.getId()[0] < s.getId()[0])//vince chi ha il site id minore
					return true;
				else
					return false;
			}

			return false;
			});

		if (it != _symbols.end()) {

			if (it == _symbols.begin()) {
				index = 0;
			}
			else {

				index = std::distance(_symbols.begin(), it);//mi dice la posizione del carattere nel crdt ossia dove sono in relazione 
			   //all'inizio della Qstring che rappresenta il testo qui al contarario di prima ritorno solo se ho trovato 
			   //altrimenti non devo fare nulla-->segnalato da -1 che � gestito nel process
			}
			_symbols.insert(it, symbol);

		}
	}

	if ((unsigned)index < _symbols.size())
		return index;
	else
		return (index - 1);//non so se va messo o basta ritornare sempre index fare prove 
}

__int64 CRDT::delete_symbol(Symbol symbol)
{
	__int64 index;

	auto it = std::find_if(this->_symbols.begin(), this->_symbols.end(),
		[symbol](Symbol s) {return ((symbol.getId() == s.getId())); });

	if (it != _symbols.end()) {


		if (it == _symbols.begin()) {
			index = 0;
		}
		else {

			index = std::distance(_symbols.begin(), it);//mi dice la posizione del carattere nel crdt ossia dove sono in relazione 
		   //all'inizio della Qstring che rappresenta il testo qui al contarario di prima ritorno solo se ho trovato 
		   //altrimenti non devo fare nulla-->segnalato da -1 che � gestito nel process
		}

		//vuol dire che l'ho trovato
		_symbols.erase(it);

		return index;
	}

	return -1;
}
__int64 CRDT::change_symbol(Symbol symbol) {
	__int64 index;

	auto it = std::find_if(this->_symbols.begin(), this->_symbols.end(),
		[symbol](Symbol s) {return ((s.getPos() == symbol.getPos()) && (symbol.getId() == s.getId())); });


	if (it != _symbols.end()) {
		//vuol dire che l'ho trovato
		(it)->setFont(symbol.getFont());
		(it)->setColor(symbol.getColor());
		(it)->setAlignment(symbol.getAlignment());

		index = std::distance(_symbols.begin(), it);


		return index;
	}

	return -1;

}

__int64 CRDT::process(const Message& m)
{
	__int64 index = 0;

	switch (m.getAction())
	{
	case DELETE_S:
		index = delete_symbol(m.getSymbol());
		//fare qualcosa con index
		break;
	case INSERT:
		index = insert_symbol(m.getSymbol());
		//fare qualcosa con index
		break;
	case CHANGE:
		index = change_symbol(m.getSymbol());
		break;

	default:
		index = 0;
		break;
	}

	if (index == -1) {
		std::cout << "ERRORE CON GLI INDICI NELLA PROCESS";
	}
	return index;
}


//local insert/delete di conclave creano il messaggio che identifica le mie azioni e creo anche qui la parte frazionaria ecc
Message CRDT::localInsert(int index, char value, QFont font, QColor color, Qt::AlignmentFlag alignment)
{
	std::array<int, 2> a;
	std::vector<int> pos;
	//se � vuoto ha pos 0
	if (_symbols.empty()) {
		pos.push_back(0);
		Symbol s(value, a = { this->_siteId,_counter++ }, pos, font, color, alignment);
		_symbols.push_back(s);

		//mando il messaggio
		Message m(s, INSERT, this->_siteId);
		return m;
	}
	//caso particolare se inserisco in zero
	if (index == 0) {
		int tmp = _symbols.at(index).getPos()[0];
		pos.push_back(tmp - 1);


		Symbol s(value, a = { this->_siteId,_counter++ }, pos, font, color, alignment);
		_symbols.insert(_symbols.begin() + index, s);

		//mando il messaggio
		Message m(s, INSERT, this->_siteId);

		return m;

	}

	if ((unsigned)index < _symbols.size()) {
		//devo trovare l'indice frazionario 
		std::vector<int> previous = _symbols.at(index - 1).getPos();
		std::vector<int> curr = _symbols.at(index).getPos();

		if ((curr.size() <= previous.size()) /*1st version||( (previous.at(0) < 0))&&(curr.size() <= previous.size())*/) {
			//metto 5 in fondo
			pos = previous;
			if (curr.size() < previous.size()) {
				pos.at(pos.size() - 1)++;
			}
			else {
				pos.push_back(5);
			}

		}
		else {
			//metto 1 nella penultima-->1st version
			pos = curr;
			//pos.insert(pos.end() - 1, 1);//end ritorna il primo NON elemento
			pos.at(pos.size() - 1)--;


			//metto 1 in coda a previous � maggiore di previous ma  minore di current ugualmente
			//pos = previous;//O(N)
			//pos.push_back(1);//O(1)
		}

		Symbol s(value, a = { this->_siteId,_counter++ }, pos, font, color, alignment);
		_symbols.insert(_symbols.begin() + index, s);


		//mando il messaggio
		Message m(s, INSERT, this->_siteId);


		return m;

	}
	else {
		if ((unsigned)index == _symbols.size()) {
			//inserisco in coda
			int new_index = _symbols.back().getPos().at(0) + 1;
			pos.push_back(new_index);
			Symbol s(value, a = { this->_siteId,_counter++ }, pos, font, color, alignment);
			_symbols.push_back(s);
			//mando il messaggio
			Message m(s, INSERT, this->_siteId);

			//QTextStream(stdout) << font.toString() << endl;

			return m;

		}
		else {
			//lancia eccezione ho sforato
			throw std::length_error("Errore indice--> inserito ad indice errato");
		}
	}
}

Message CRDT::localErase(int index)
{
	if ((unsigned)index > _symbols.size()) {
		throw std::length_error("Indice errato\n");
	}
	//trovo il simbolo
	Symbol s = _symbols.at(index);
	//quello che elimino � per me un nuovo carattere


	//mando il messaggio
	Message m(s, DELETE_S, this->_siteId);

	//elimino il simbolo dal vettore locale
	_symbols.erase(_symbols.begin() + index);

	return m;
}

Message CRDT::localChange(int index, char value, QFont font, QColor color, Qt::AlignmentFlag alignment)
{
	_symbols.at(index).setColor(color);
	_symbols.at(index).setFont(font);
	_symbols.at(index).setAlignment(alignment);
	Symbol s = _symbols.at(index);
	Message m(s, CHANGE, this->_siteId);
	return m;
}


std::string CRDT::to_string()
{
	std::string str;
	for (Symbol s : _symbols)
		str.push_back(s.getChar());

	return str;
}

int CRDT::getId()
{
	return this->_siteId;
}

Symbol CRDT::getSymbol(int index)
{
	if (index < this->_symbols.size())
		return this->_symbols.at(index);
	//caso in cui viene mandata la posizione del cursore agli altri client senza alcun carattere inserito,
	//oppure viene mandata agli altri client la posizione del cursore quando si trova dopo l'ultimo carattere inserito
	else {
		std::vector<int> pos;
		pos.push_back(INT_MAX);
		return Symbol(pos);
	}
}



std::vector<Message> CRDT::getMessageArray()
{
	std::vector<Message> msgs;
	for (Symbol s : this->_symbols) {

		Message m(s, INSERT, -1);//il server ha id -1
		msgs.push_back(m);
	}

	return msgs;
}




// SERVER ONLY-----------------------------
QJsonObject ObjectFromString(const QString& in)
{
	QJsonObject obj;

	QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

	// check validity of the document
	if (!doc.isNull())
	{
		if (doc.isObject())
		{
			obj = doc.object();
		}
		else
		{
			std::cout << "Document is not an object" << std::endl;
		}
	}
	else
	{
		std::cout << "Invalid JSON...\n" << in.toStdString() << std::endl;
	}

	return obj;
}
std::vector<Message> CRDT::readFromFile(std::string fileName)//NON USARE ANCORA MODIFICHE DA FARE-->MATTIA--> TOGLIERE LA LISTA DI MESSAGGI USATA PER TESTARE IL CLIENT
{
	std::ifstream iFile(fileName);
	std::vector<Symbol> local_symbols;
	if (iFile.is_open())
	{
		std::string line;


		while (getline(iFile, line))
		{
			QString str = QString::fromStdString(line);
			QJsonObject  obj = ObjectFromString(str);

			char c = obj.value("character").toInt();

			std::array<int, 2> a;
			QJsonArray id = obj.value("globalCharacterId").toArray();
			a[0] = id[0].toInt();
			a[1] = id[1].toInt();

			std::vector<int> pos;

			QJsonArray vett_pos = obj.value("position").toArray();

			for (QJsonValue qj : vett_pos) {

				pos.push_back(qj.toInt());
			}

			QFont font;
			font.fromString(obj.value("font").toString());


			QString color_hex = obj.value("color").toString();

			QColor color(color_hex);


			int align = obj.value("alignment").toInt();
			Qt::AlignmentFlag alignFlag = static_cast<Qt::AlignmentFlag>(align);


			Symbol s(c, a, pos, font, color, alignFlag);


			this->_symbols.push_back(s);

			//per fare prove
			//local_symbols.push_back(s);
		}


		iFile.close();
		std::vector<Message> local_m;
		//prima carico tutto e poi inizio a mandare i messaggi
		for (auto symb : this->_symbols) {

			Message m(symb, CHANGE, 0);//L'ID del server � 0 sempre
			local_m.push_back(m);


			//emit robaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
		}
		return local_m;
	}
}



QString CRDT::crdt_serialize()
{
	QString text = "";

	for (auto s : this->_symbols) {

		QJsonObject obj;


		char c = s.getChar();

		obj.insert("character", QJsonValue(c));

		//include di vector e array sono gia in symbol includso in message
		std::array<int, 2> id = s.getId();

		QJsonArray  Qid = { id[0],id[1] };//id globale

		obj.insert("globalCharacterId", QJsonValue(Qid));

		std::vector<int> v = s.getPos();

		QJsonArray Qvett;

		for (int i : v) {

			Qvett.append(QJsonValue(i));
		}

		obj.insert("position", QJsonValue(Qvett));


		//font e colore
		QFont font = s.getFont();
		QString serialFont = font.toString();
		obj.insert("font", QJsonValue(serialFont));

		QString color = s.getColor().name();//hex value
		Qt::AlignmentFlag aligment = s.getAlignment();

		//int red = color.red();
		//int green = color.green();
		//int blue = color.blue();

		//obj.insert("red",QJsonValue(red));
		//obj.insert("green", QJsonValue(green));
		//obj.insert("blue", QJsonValue(blue));
		obj.insert("color", color);
		obj.insert("alignment", aligment);


		QJsonDocument doc(obj);
		QString strJson(doc.toJson(QJsonDocument::Compact));
		text.append(strJson);
		text.append('\n');
	}
	return text;
}
void CRDT::saveOnFile(std::string filename)
{

	QString serialized_text = this->crdt_serialize();

	std::ofstream oFile(filename, std::ios_base::out | std::ios_base::trunc);
	if (oFile.is_open())
	{

		//std::string text = this->to_string();
		{
			//oFile << text;
			oFile << serialized_text.toStdString();
		}
		oFile.close();
	}
	else {
		std::cout << "Errore apertura file";
	}

}

bool CRDT::isEmpty()
{
	return this->_symbols.size() == 0;
}

__int64 CRDT::getCursorPosition(std::vector<int> crdtPos) {
	__int64 index = -1;


	if (this->_symbols.empty()) {
		//inserisco in coda, lo faccio come prima operazione in modo da ottimizzare l'inserimento
		//quando carico dal server
		index = 0;
	}
	else if (crdtPos > this->_symbols.back().getPos()) {
		index = this->_symbols.size();
	}
	else {

		//trovo l'iteratore che punta alla posizione in cui inserire basandomi sulle pos frazionarie
		auto it = std::find_if(this->_symbols.begin(), this->_symbols.end(), [crdtPos](Symbol s) {

			if (s.getPos() >= crdtPos)
				return true;
			/*else if (s.getPos() == crdtPos) {

				if (symbol.getId()[0] < s.getId()[0])//vince chi ha il site id minore
					return true;
				else
					return false;
			}*/

			return false;
			});

		if (it != _symbols.end()) {

			index = std::distance(_symbols.begin(), it);//mi dice la posizione del carattere nel crdt ossia dove sono in relazione 
		   //all'inizio della Qstring che rappresenta il testo qui al contarario di prima ritorno solo se ho trovato 
		   //altrimenti non devo fare nulla-->segnalato da -1 che � gestito nel process


		}
	}

	if ((unsigned)index <= _symbols.size())
		return index;
	else
		return (index - 1);//non so se va messo o basta ritornare sempre index fare prove 
}

void CRDT::updateUserInterval() {
	m_usersInterval.clear();
	int start = 0;
	int end = 0;
	int userId = -1;
	for (auto it = _symbols.begin(); it != _symbols.end(); it++) {
		if (it->getId()[0] != this->_siteId) {
			userId = it->getId()[0];
			end++;
			if ((it + 1) != _symbols.end() && userId != (it + 1)->getId()[0]) {
				m_usersInterval.emplace_back(userId, start, end);
				start = end;
			}
		}
		else {
			start++;
			end = start;
		}

		//start = end;
		//do {
		//	userId = it->getId()[0];
		//	end++;
		//	it++;
		//	if (it == _symbols.end()) break;
		//} while (it->getId()[0] == userId);
		//m_usersInterval.emplace_back(userId, start, end);
	}
	m_usersInterval.emplace_back(userId, start, end);
}

void CRDT::setSiteCounter(int siteCounter) {
	this->_counter = siteCounter;
}
