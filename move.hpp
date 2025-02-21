#ifndef MOVE_HPP
#define MOVE_HPP
#include "square.hpp"

class Move
{
	private:
		Square start;
		Square end;
		char piece;
		int turn;
		int color;
		int score;
		char capture;
		bool queenscastle;
		bool kingscastle;
		bool enpassant;
		bool check;
		bool checkmate;
		bool promotion;
		char promotionpiece;
	public:
		Move();
		Move(int turn, int color, char piece, Square start, Square end, char capture = ' ', bool kingscastle = false, bool queenscastle = false, bool enpassant = false, bool check = false, bool checkmate = false, bool promotion = false, char promotionpiece = ' ', int score = 0);
		Square getStart();
		Move& operator=(const Move& move);
		bool operator==(Move& move2);
		Square getEnd();
		int getScore();
		void setScore(int score);
		char getPiece();
		char getCapture();
		bool getPromotion();
		char getPromotionPiece();
		int getColor();
		int getTurn();
		bool getKingsCastle();
		bool getQueensCastle();
		bool getEnPassant();
		bool getCheck();
		bool getCheckmate();
		void setCheck(bool check);
		void setCheckmate(bool checkmate);
		void setPromotion(bool promotion);
		void setPromotionPiece(char promotionpiece);
};


#endif
