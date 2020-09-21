#pragma once
#include "Symbol.h"


class Message
{
private:
	Symbol symbol;
	int action = -1;
	int sender = -1;
	__int64 cursor_position = -1;
	std::vector<std::vector<int>> idList;
	
public:
	Message(Symbol s, int action,int sender);
	Message(__int64 position, int action, int sender);
	Message(std::vector<std::vector<int>> idList, int action, int sender);
	~Message();
	Symbol getSymbol() const;
	int getAction() const;
	int getSenderId();
	std::vector<std::vector<int>> getListIds();
	__int64 getCursorPosition();
};

