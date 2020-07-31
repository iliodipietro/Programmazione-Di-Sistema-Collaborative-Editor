#include "Symbol.h"


Symbol::Symbol(char character, std::array<int, 2> id, std::vector<int> vett, QFont font, QColor color, Qt::AlignmentFlag alignment)
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

Qt::AlignmentFlag Symbol::getAlignment()
{
	return this->alignment;
}

void Symbol::setFont(QFont font)
{
	this->font = font;
}

void Symbol::setColor(QColor color)
{
	this->color = color;
}

void Symbol::setAlignment(Qt::AlignmentFlag al)
{
	this->alignment = al;
}
