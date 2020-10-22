#pragma once
#include <vector>
#include <exception>
#include "Symbol.h"
#include "Message.h"
#include <fstream>
#include <string>
#include <iostream>
#include <QTimer>
#include <QObject>
#include <QFile>
#include <QDebug>

//define per azioni
#define INSERT_SYMBOL 0
#define DELETE_SYMBOL 1
#define CHANGE_SYMBOL 2
#define CURSOR_S 3


//altre define
#define TIMEOUT 2000// dopo quanto tempo il crdt deve essere salvato su file
//#define TIMEOUT 10000 // 10 secondi

class Message;

/*versione con modifica del crdt lineare -->MATRICE SE SI E' DEI FOLLI MA POSSIBILE

le local insert/delete ritornano dei messaggi che dovranno essere presi serializzati e mandati tramite socket al server

L'algoritmo è quello del lab e funzionava non so se il tutto funziona adesso dopo i cambiamneti -->FARE PROVE QUANDO SI SARA
IMPLEMENTATO UN MECCANISMO FUNZIONANTE


*/
class CRDT: public QObject
{
	Q_OBJECT
private:

	int _siteId;
	int _counter;
	std::vector<Symbol> _symbols;
	//uso __int64 per evitare warning e perdita dati per troncamento
	__int64 insert_symbol(Symbol symbol);
	__int64 delete_symbol(Symbol symbol);
	__int64 change_symbol(Symbol symbol);
	QString crdt_serialize();
	QString path;//path che mi dice dove salvare il file ogni volta che scade il timer
	QTimer* timer;

public:
	CRDT(int id, QString path);//vuole l'id e un path su cui si andrà nel caso a salvare il file
	~CRDT();

	Message localInsert(int index, char value, QFont font, QColor color, Qt::AlignmentFlag alignment);
	Message localErase(int index);
	Message localChange(int index, char value, QFont font, QColor color, Qt::AlignmentFlag alignment);

	__int64 process(const Message& m);
	std::string to_string();//usare Qstring??
	int getId();
	bool isEmpty();
	//SERVER ONLY
	std::vector<Message> getMessageArray();//SERVER ONLY-->questo vettore va mandato con un for ai socket con all'interno un serializzatore mando i messaggi uno alla volta
	//std::vector<Message> readFromFile();
	void readFromFile();

	QTimer* getTimer();

    std::vector<int> getNextCursorPosition(std::vector<int> crdtPos);
    std::vector<int> fromIteratorToPosition(std::vector<Symbol>::iterator it);

public slots:
	void saveOnFile();//versione base salva solo i caratteri e non il formato--> da testare



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

	void printText() {

		QString str;

		for (Symbol s : _symbols) {

			char c = s.getChar();
			str.append(c);

		}

		qDebug() << str << '\n';
	}
};

