#pragma once
#include <vector>
#include <exception>
#include "Symbol.h"
#include "Message.h"
#include "Editor/UserInterval.h"
#include <fstream>
#include <string>
#include <iostream>



#define INSERT 0
#define DELETE_S 1
#define CHANGE 2
#define CURSOR_S 3
class Message;

class CRDT
{
private:

	int _siteId;
	int _counter;
	std::vector<Symbol> _symbols;
	std::vector<UserInterval> m_usersInterval;
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
	Message localChange(int index, char value, QFont font, const QColor color, Qt::AlignmentFlag alignment);

	__int64 process(const Message& m);
	std::string to_string();//usare Qstring??
	int getId();
	Symbol getSymbol(int index);
	//SERVER ONLY
	//void dispatchMessages();-->sul server
	std::vector<Message> getMessageArray();//SERVER ONLY-->questo vettore va mandato con un for ai socket con all'interno un serializzatore mando i messaggi uno alla volta
	std::vector<Message> readFromFile(std::string fileName);
	void saveOnFile(std::string filename);//versione base salva solo i caratteri e non il formato--> da testare
	bool isEmpty();
	void updateUserInterval();
	inline std::vector<UserInterval>* getUsersInterval() { return &m_usersInterval; };
	void setSiteCounter(int siteCounter);
	inline int getSiteCounter() { return this->_counter; };

	__int64 getCursorPosition(std::vector<int> crdtPos);

	//for fractional position debug only
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

