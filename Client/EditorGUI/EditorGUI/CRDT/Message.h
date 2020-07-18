#pragma once
#include "Symbol.h"
#define INSERT 0
#define DELETE 1
#define CHANGE 2//change in the style
class Message
{
private:
	Symbol symbol;
	int action;
	int sender;
	
public:
	Message(Symbol s, int action,int sender);
	~Message();
	Symbol getSymbol() const;
	int getAction() const;
	int getSenderId();
};

