#include "square.hpp"

#include <iostream>
#include <sstream>

Square::Square()
{
	file = ' ';
	row = 0;
	empty = true;
	piece = ' ';
	color = -1;
}

Square::Square(char file, int row)
{
	this->file = file;
	this->row = row;
	empty = true;
	piece = ' ';
	color = -1;
	attackers = {' '};
	defenders = {' '};
	//defendersquares = {0};
	//attackersquares = {0};
}

void Square::deletePiece()
{
	piece = ' ';
	color = -1;
	empty = true;
}

void Square::resetDefense()
{
	defense = 0;
	while(defenders.size() > 0)
	{
		defenders.pop_back();
	}
	while(defendersquares.size() > 0)
	{
		defendersquares.pop_back();
	}
		
}

void Square::resetAttacks()
{
	attacks = 0;
	while(attackers.size() > 0)
	{
		attackers.pop_back();
	}
	while(attackersquares.size() > 0)
	{
		attackersquares.pop_back();
	}
}

void Square::addAttacker(char attacker, int attackersquare)
{
	this->attackers.push_back(attacker);
	this->attackersquares.push_back(attackersquare);
}

void Square::addDefender(char defender, int defendersquare)
{
	this->defenders.push_back(defender);
	this->defendersquares.push_back(defendersquare);
}

std::vector<int> Square::getAttackerSquares()
{
	return attackersquares;
}

std::vector<int> Square::getDefenderSquares()
{
	return defendersquares;
}

void Square::addDefense()
{
	defense++;
}

void Square::addAttack()
{
	attacks++;
}

bool Square::IsEmpty()
{
	return empty;
}

int Square::getAttack()
{
	return attacks;
}

int Square::getDefense()
{
	return defense;
}

std::vector<char> Square::getAttackers()
{
	return attackers;
}

std::vector<char> Square::getDefenders()
{
	return defenders;
}

void Square::setSquare(char file, int row, int rowindex, int columnindex)
{
	this->rowindex = rowindex;
	this->columnindex = columnindex;
	this->file = file;
	this->row = row;
}

void Square::setPiece(char piece, int color)
{
	empty = false;
	this->piece = piece;
	this->color = color;
}

int Square::getColor()
{
	return color;
}

char Square::getPiece()
{
	return piece;
}

std::string Square::SquareName()
{
	std::stringstream stream;
	stream << file;
	stream <<  row;
	return stream.str();
}

char Square::getFile()
{
	return file;
}

int Square::getRow()
{
	return row;
}

int Square::getRowIndex()
{
	return rowindex;
}

int Square::getColumnIndex()
{
	return columnindex;
}

Square& Square::operator=(const Square& square)
{
	this->file = square.file;
	this->row = square.row;
	this->rowindex = square.rowindex;
	this->columnindex = square.columnindex;
	this->empty = square.empty;
	this->piece = square.piece;
	this->color = square.color;
	this->attacks = square.attacks;
	this->defense = square.defense;
	//std::cout << "set square '" << SquareName() << "'. About to set vectors.." << std::endl;
	this->attackers = square.attackers;
	//std::cout << "We set attackers to successfully. There are " << attackers.size() << " attackers according to the attacker vector size. " << std::endl;
	this->defenders = square.defenders;
	//this->defendersquares = square.defendersquares;
	//this->attackersquares = square.attackersquares;
	return *this;
}

bool Square::operator==(Square& square2)
{
	return this->file == square2.file && this->row == square2.row;
}

Square::Square(const Square& square)
{
	this->file = square.file;
        this->row = square.row;
        this->rowindex = square.rowindex;
        this->columnindex = square.columnindex;
        this->empty = square.empty;
	this->attacks = square.attacks;
	this->defense = square.defense;
	this->piece = square.piece;
	this->color = square.color;
	this->attackers = square.attackers;
	this->defenders = square.defenders;
	//this->attackersquares = square.attackersquares;
	//this->defendersquares = square.defendersquares;

}
