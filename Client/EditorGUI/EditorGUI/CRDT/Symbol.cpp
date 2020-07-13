#include "Symbol.h"


Symbol::Symbol(char character, std::array<int, 2> id, std::vector<int> vett, QFont font, QColor color, Qt::Alignment alignment)
    :character(character), identifier(id), position(vett), font(font), color(color), alignment(alignment)
{
}

Symbol::~Symbol()
{
}

char Symbol::getChar()
{
	return character;
}

std::array<int, 2> Symbol::getId()const
{
	return identifier;
}

std::vector<int> Symbol::getPos() const
{
	return position;
}
QFont Symbol::getFont()
{
	return this->font;
}

QColor Symbol::getColor()
{
	return this->color;
}

Qt::Alignment Symbol::getAlignment(){
    return this->alignment;
}
