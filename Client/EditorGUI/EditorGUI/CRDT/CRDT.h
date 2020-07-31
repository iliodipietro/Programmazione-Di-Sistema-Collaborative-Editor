#pragma once
#include <vector>
#include <exception>
#include "Symbol.h"
#include "Message.h"
#include <fstream>
#include <string>
#include <iostream>
#include <QTextStream>

#define INSERT 0
#define DELETE 1



class Message;

/*versione con modifica del crdt lineare -->MATRICE SE SI E' DEI FOLLI MA POSSIBILE

le local insert/delete ritornano dei messaggi che dovranno essere presi serializzati e mandati tramite socket al server

L'algoritmo � quello del lab e funzionava non so se il tutto funziona adesso dopo i cambiamneti -->FARE PROVE QUANDO SI SARA
IMPLEMENTATO UN MECCANISMO FUNZIONANTE



TODO
mancano le interazioni con la gui ossia i signal e slot per scatenare le insert. 
la process deve in qualche modo andare a modificare il testo????--> per ora ho solo un intero che dice a quale posizione 
dall'inizio del vettore si trova il carattere interessato
*/
class CRDT
{
private:

	int _siteId;
	int _counter;
	std::vector<Symbol> _symbols;
	//uso __int64 per evitare warning e perdita dati per troncamento
	__int64 insert_symbol(Symbol symbol);
	__int64 delete_symbol(Symbol symbol);

public:
	CRDT(int id);
	~CRDT();

    Message localInsert(int index, char value, QFont font, QColor color, Qt::AlignmentFlag alignment);
	Message localErase(int index);
	__int64 process(const Message & m);
	std::string to_string();//usare Qstring??
	int getId();
	//SERVER ONLY
	//void dispatchMessages();-->sul server
	std::vector<Message> getMessageArray();//SERVER ONLY-->questo vettore va mandato con un for ai socket con all'interno un serializzatore mando i messaggi uno alla volta
	void readFromFile(std::string fileName);
	void writeToFile(std::string filename);//versione base salva solo i caratteri e non il formato--> da testare



	//for fractional position debug
	void printPositions()
	{
		for (Symbol s : _symbols) {

			std::vector<int> dv = s.getPos();
			for (int d : dv)
				std::cout << d << " ";

			std::cout << std::endl;

		}
		std::cout << std::endl;
	}
};

