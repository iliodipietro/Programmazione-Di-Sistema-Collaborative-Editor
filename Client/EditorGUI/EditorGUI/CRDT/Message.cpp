#include "Message.h"



Message::Message(Symbol s, int action,int sender):symbol(s),action(action),sender(sender),cursor_position(-1)
{
}



Message::Message(__int64 position, int action, int sender):cursor_position(position), action(action), sender(sender)
{
	this->symbol = Symbol();
}

Message::Message(std::vector<std::vector<int>> idList,int action, int sender):action(action),sender(sender)
{
	this->idList = idList;
}


Message::~Message()
{
}

Symbol Message::getSymbol() const
{
	return symbol;
}

 int Message::getAction() const
{
	return action;
}

 int Message::getSenderId()
 {
	 return this->sender;
 }

 std::vector<std::vector<int>> Message::getListIds()
 {
	 return this->idList;
 }



 __int64 Message::getCursorPosition()
 {
	 return this->cursor_position;
 }


