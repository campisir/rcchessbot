#include <string>
#include <vector>

class Square
{
	private:
		char file;
		int rowindex;
		int columnindex;
		int row;
		bool empty;
		char piece;
		int color;
		int attacks = 0;
		int defense = 0;
		std::vector<char> attackers;
		std::vector<int> attackersquares = {};
		std::vector<char> defenders;
		std::vector<int> defendersquares = {};
	public:
		Square();
		Square(char file, int row);
		Square(const Square& square);
		Square& operator=(const Square& square);
		bool operator==(Square& square2);
		void addAttacker(char attacker, int attackersquare);
		void addDefender(char defender, int defendersquare);
		std::vector<char> getDefenders();
		std::vector<char> getAttackers();
		std::vector<int> getDefenderSquares();
		std::vector<int> getAttackerSquares();
		bool IsEmpty();
		void setSquare(char file, int row, int rowindex, int columnindex);
		void setPiece(char piece, int color);
		char getPiece();
		int getColor();
		char getFile();
		void resetAttacks();
		void resetDefense();
		int getAttack();
		int getDefense();
		void addDefense();
		void addAttack();
		int getRow();
		int getRowIndex();
		int getColumnIndex();
		std::string SquareName();
		void deletePiece();

};
