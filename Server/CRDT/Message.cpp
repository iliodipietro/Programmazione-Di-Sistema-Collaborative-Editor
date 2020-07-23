#include "Message.h"



Message::Message(Symbol s, int action,int sender):symbol(s),action(action),sender(sender)
{
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


