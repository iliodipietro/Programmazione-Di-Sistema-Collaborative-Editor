#pragma once
#include <vector>
#include <exception>
#include "Symbol.h"
#include "Message.h"
#include <fstream>
#include <string>
#include <iostream>



#define INSERT 0
#define DELETE_S 1
#define CHANGE 2
#define CURSOR 3



class Message;

/*
le local insert/delete ritornano dei messaggi che dovranno essere presi serializzati e mandati tramite socket al server

L'algoritmo è quello del lab e funzionava non so se il tutto funziona adesso dopo i cambiamneti -->FARE PROVE QUANDO SI SARA
IMPLEMENTATO UN MECCANISMO FUNZIONANTE



TODO
mancano le interazioni con la gui ossia i signal e slot per scatenare le insert.
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
	__int64 change_symbol(Symbol symbol);
	QString crdt_serialize();

public:
	CRDT(int id);
	~CRDT();

	Message localInsert(int index, char value, QFont font, QColor color, Qt::AlignmentFlag alignment);
	Message localErase(int index);
	Message localChange(int index, char value, QFont font, QColor color, Qt::AlignmentFlag alignment);

	__int64 process(const Message& m);
	std::string to_string();//usare Qstring??
	int getId();
	Symbol getSymbol(int index);
	//SERVER ONLY
	//void dispatchMessages();-->sul server
	std::vector<Message> getMessageArray();//SERVER ONLY-->questo vettore va mandato con un for ai socket con all'interno un serializzatore mando i messaggi uno alla volta
	std::vector<Message> readFromFile(std::string fileName);
	void saveOnFile(std::string filename);//versione base salva solo i caratteri e non il formato--> da testare



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

