#include "move.hpp"

Move::Move()
{
}

Move::Move(int turn, int color, char piece, Square start, Square end, char capture, bool kingscastle, bool queenscastle, bool enpassant, bool check, bool checkmate, bool promotion, char promotionpiece, int score)
{
	this->turn = turn;
	this->color = color;
	this->piece = piece;
	this->start = start;
	this->end = end;
	this->capture = capture;
	this->kingscastle = kingscastle;
	this->queenscastle = queenscastle;
	this->enpassant = enpassant;
	this->check = check;
	this->checkmate = checkmate;
	this->promotion = promotion;
	this->promotionpiece = promotionpiece;
	this->score = score;
}

Move& Move::operator=(const Move& move)
{
	this->turn = move.turn;
	this->color = move.color;
	this->piece = move.piece;
	this->start = move.start;
	this->end = move.end;
	this->capture = move.capture;
	this->kingscastle = move.kingscastle;
	this->queenscastle = move.queenscastle;
	this->enpassant = move.enpassant;
	this->check = move.check;
	this->checkmate = move.checkmate;
	this->promotion = move.promotion;
	this->promotionpiece = move.promotionpiece;
	this->score = move.score;
	return *this;
}

bool Move::operator==(Move& move2)
{
	return color == move2.color && piece == move2.piece && start.SquareName() == move2.start.SquareName() && end.SquareName() == move2.end.SquareName() && capture == move2.capture;
}

void Move::setScore(int score)
{
	this->score = score;
}

int Move::getScore()
{
	return score;
}

Square Move::getStart()
{
	return start;
}

Square Move::getEnd()
{
	return end;
}

void Move::setCheck(bool check)
{
	this->check = check;
}

void Move::setCheckmate(bool checkmate)
{
	this->checkmate = checkmate;
}

void Move::setPromotion(bool promotion)
{
	this->promotion = promotion;
}

void Move::setPromotionPiece(char promotionpiece)
{
	this->promotionpiece = promotionpiece;
}

bool Move::getPromotion()
{
	return promotion;
}

char Move::getPromotionPiece()
{
	return promotionpiece;
}

bool Move::getCheck()
{
	return check;
}

bool Move::getCheckmate()
{
	return checkmate;
}

char Move::getPiece()
{
	return piece;
}

char Move::getCapture()
{
	return capture;
}

int Move::getColor()
{
	return color;
}

int Move::getTurn()
{
	return turn;
}

bool Move::getKingsCastle()
{
	return kingscastle;
}

bool Move::getQueensCastle()
{
	return queenscastle;
}

bool Move::getEnPassant()
{
	return enpassant;
}
