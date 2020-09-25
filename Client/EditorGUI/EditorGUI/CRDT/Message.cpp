#include "Message.h"



Message::Message(Symbol s, int action,int sender):symbol(s),action(action),sender(sender),cursor_position(-1)
{
}



Message::Message(std::vector<int> position, int action, int sender, bool isSelection):cursor_position(position), action(action), sender(sender), isSelection(isSelection)
{
	this->symbol = Symbol();
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

 std::vector<int> Message::getCursorPosition()
 {
	 return this->cursor_position;
 }

 bool Message::getIsSelection() {
     return this->isSelection;
 }
