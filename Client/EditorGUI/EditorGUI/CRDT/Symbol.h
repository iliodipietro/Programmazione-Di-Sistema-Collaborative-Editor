#pragma once
#include<iostream>
#include<vector>
#include<array>
#include<QFont>
#include<QColor>
#include "../Structures/FormatStructure.h"//??

class Symbol
{
private:
	char character;
	std::array<int, 2> identifier;
	std::vector<int> position;

	QFont font;
	QColor color;
	Qt::AlignmentFlag alignment;
	//FormatStructure FS;

public:
    Symbol(char character, std::array<int, 2> id, std::vector<int> vett, QFont font, QColor color, Qt::AlignmentFlag alignment);
	~Symbol();
	char getChar();
	std::array<int, 2> getId() const;
	std::vector<int> getPos() const;
	QFont getFont();
	QColor getColor();
	Qt::AlignmentFlag getAlignment();
	void setFont(QFont font);
	void setColor(QColor color);
	void setAlignment(Qt::AlignmentFlag al);
};

