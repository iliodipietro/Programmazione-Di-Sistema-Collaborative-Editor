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
	__int64 cursor_position;
	
public:
	Message(Symbol s, int action,int sender);
	Message(__int64 position, int action, int sender);
	~Message();
	Symbol getSymbol() const;
	int getAction() const;
	int getSenderId();
	__int64 getCursorPosition();
};

