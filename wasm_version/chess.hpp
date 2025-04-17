#include "move.hpp"
#include <vector>
#include <sstream>
class Chess
{
	private:
		Square** board;
		std::vector<Move> log;
		std::string forcemove;
		bool stalemate = false;
		bool checkmate = false;
		bool check = false;
		bool blackcheck = false;
		int turn = 0;
		bool enpassant = false;
		int enpassantcol;
		bool cancastlekingside = true;
		bool cancastlequeenside = true;
		bool canblackcastlekingside = true;
		bool canblackcastlequeenside = true;
		bool veryspecificexception = false;
		bool fiftymoverule = false;
		bool repetition = false;
		bool lackmaterial = false;
		std::stringstream scores;
		std::stringstream thoughts;
	public:
		Chess();
		void SetBoard();
		void UndoMove();
		void Help();
		bool HasAValidMove();
		bool DidIWin(bool checkfurther = false, bool mateintwo = false, bool blackmateintwo = false);
		void PromotionZone();
		bool BlacksMove(std::string move);
		bool ValidMove(int row, int col, char piece, int color, char file = ' ', bool makemove = true);
		void PrintBoard();
		void WhitetoMove();
		void BlacktoMove();
		void TriggerCheck();
		void TriggerBlackCheck();
		void PrintSquares();
		void DefenseReset();
		void PrintMoves();
		bool SeeWhiteDeeper(bool passback = false);
		bool DealWithCheck(std::vector<Square>& moveable);
		int Decide(double x = 0, std::vector<Move> validmoves = {});
		int CalcScore(Move move);
		void PrintScores();
		void CheckForDraws();
		std::string MoveFromPos(std::string moves);
		std::string GetLastMove();
};
