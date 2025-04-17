#include "chess.hpp"
#include <iostream>
#include <vector>
#include <time.h>
#include <fstream>

Chess::Chess()
{
}

std::string Chess::GetLastMove()
{
	if (log.empty())
		return "";

	Move m = log.back();
	std::stringstream ss;

	ss << m.getStart().SquareName() << m.getEnd().SquareName();

	if (m.getPromotion())
	{
		ss << "=" << char(toupper(m.getPromotionPiece()));
	}

	return ss.str();
}

std::string Chess::MoveFromPos(std::string moves)
{
    srand((unsigned)time(NULL));
    int ascii = 97;
    board = new Square*[8];
    board = new Square*[8];
	for(int i = 0; i < 8; i++)
	{
		board[i] = new Square[8];
	}
	for(int i = 0; i < 8; i++)
	{
		board[0][i].setSquare(ascii, 1, 0, i);
		board[1][i].setSquare(ascii, 2, 1, i);
		board[2][i].setSquare(ascii, 3, 2, i);
		board[3][i].setSquare(ascii, 4, 3, i);
		board[4][i].setSquare(ascii, 5, 4, i);
		board[5][i].setSquare(ascii, 6, 5, i);
		board[6][i].setSquare(ascii, 7, 6, i);
		board[7][i].setSquare(ascii, 8, 7, i);
		ascii++;
    }

    SetBoard();

    std::istringstream iss(moves);
    std::string token;

    iss >> token;
    enpassant = (token == "1");
    iss >> token;
    enpassantcol = std::stoi(token);
    iss >> token;
    cancastlekingside = (token == "1");
    iss >> token;
    cancastlequeenside = (token == "1");
    iss >> token;
    veryspecificexception = (token == "1");

    std::string move;
    bool turnswap = false;
    turn++;
	while (iss >> move) {
		if (turnswap)
			turn++;
		turnswap = !turnswap;
		int startRow = (move[1] - '0') - 1;
		int startCol = move[0] - 'a';
		int endRow = (move[3] - '0') - 1;
		int endCol = move[2] - 'a';

		// Ensure indices are within bounds
		if (startRow < 0 || startRow >= 8 || startCol < 0 || startCol >= 8 ||
			endRow < 0 || endRow >= 8 || endCol < 0 || endCol >= 8) {
			continue;
		}

		char piece = board[startRow][startCol].getPiece();
		int color = board[startRow][startCol].getColor();

		// Handle castling
		if (piece == 'k' && abs(startCol - endCol) == 2) {
			if (endCol == 6) { // Kingside castling
				board[endRow][endCol].setPiece(piece, color);
				board[startRow][startCol].deletePiece();
				board[endRow][5].setPiece('r', color);
				board[endRow][7].deletePiece();
			} else if (endCol == 2) { // Queenside castling
				board[endRow][endCol].setPiece(piece, color);
				board[startRow][startCol].deletePiece();
				board[endRow][3].setPiece('r', color);
				board[endRow][0].deletePiece();
			}
		} else {
			// Handle en passant
			if (piece == 'p' && abs(startCol - endCol) == 1 && board[endRow][endCol].IsEmpty()) {
				board[endRow][endCol].setPiece(piece, color);
				board[startRow][startCol].deletePiece();
				board[startRow][endCol].deletePiece(); // Remove the captured pawn
			} else {
				board[endRow][endCol].setPiece(piece, color);
				board[startRow][startCol].deletePiece();
			}
		}

		// Handle promotion
		if (move.length() == 6 && move[4] == '=') {
			char promotionPiece = move[5];
			board[endRow][endCol].setPiece(promotionPiece, color);
			log.back().setPromotion(true);
			log.back().setPromotionPiece(promotionPiece);
		}

		Move m(turn, color, piece, board[startRow][startCol], board[endRow][endCol]);
		log.push_back(m);
	}

	//PrintSquares();

    scores.str("");
    thoughts.str("");

    DefenseReset();
    CheckForDraws();
    WhitetoMove();
    PromotionZone();
    CheckForDraws();

    if(checkmate)
        thoughts << std::endl << "You win by checkmate!" << std::endl << std::endl;
    else if(stalemate || (DidIWin() && !blackcheck))
        thoughts << std::endl << "It's a draw by stalemate!" << std::endl << std::endl;
    else if(DidIWin())
        thoughts << std::endl << "I win by checkmate!" << std::endl << std::endl;
    if(blackcheck)
        thoughts << "CHECK!!!" << std::endl;
	
	if(enpassant)
		enpassant = !enpassant;

	thoughts << enpassant << " " << enpassantcol << " " << cancastlekingside << " " << cancastlequeenside << " " << veryspecificexception << std::endl;	
	thoughts << "OFFICIAL MOVE: " << GetLastMove() << std::endl;
    return thoughts.str();
}

void Chess::PrintScores()
{
	thoughts << scores.str();

}

void Chess::PrintMoves()
{
	int blackwhite = 0;
	for(Move m : log)
	{
		if(blackwhite == 2)
			blackwhite = 0;
		if(blackwhite == 1)
			thoughts << "   ";
		if(blackwhite == 0)
			thoughts << m.getTurn() << ". ";
		if(m.getPiece() == 'p')
		{
			if(m.getCapture() != ' ')
			{
				thoughts << m.getStart().getFile() << "x" << m.getEnd().SquareName();
			}
			else
			{
				thoughts << m.getEnd().SquareName();
			}
			if(m.getPromotion())
			{
				thoughts << "=" << char(toupper(m.getPromotionPiece()));
			}
		}
		else if(m.getQueensCastle())
		{
			thoughts << "0-0-0";
		}
		else if(m.getKingsCastle())
		{
			thoughts << "0-0";
		}
		else
		{
	        	thoughts << char(toupper(m.getPiece()));
		        if(m.getCapture() != ' ')
			{
				thoughts << "x";
			}	
			thoughts << m.getEnd().SquareName();
		}
		if(m.getCheckmate())
		{
			thoughts << "#";
		}
		else if(m.getCheck())
		{
			thoughts << "+";
		}
		thoughts << std::endl;
		blackwhite++;
	}
}

void Chess::PrintBoard()
{
	int row = 8;
	for(int i = 7; i >= 0; i--)
	{
		std::cout << row << " |";
		for(int j = 0; j < 8; j++)
		{
			if(board[i][j].getColor() == 0)
				std::cout << board[i][j].getPiece() << " ";
			else if(board[i][j].getColor() == 1)
				std::cout << char(toupper(board[i][j].getPiece())) << " ";
			else
				std::cout << "  ";
		}
		std::cout << std::endl;
		row--;
	}
	std::cout << "   ";
	for(int i = 1; i <= 8; i++)
	{
		std::cout << "_ ";
	}
	std::cout << std::endl << "   ";
	for(int i = 65; i < 73; i++)
	{
		std::cout << char(i) << " ";
	}
	std::cout << std::endl;
}

void Chess::SetBoard()
{
	for(int i = 0; i < 8; i++)
	{
		board[1][i].setPiece('p', 0);
	}
	board[0][0].setPiece('r', 0);
	board[0][1].setPiece('n', 0);
	board[0][2].setPiece('b', 0);
	board[0][3].setPiece('q', 0);
	board[0][4].setPiece('k', 0);
	board[0][5].setPiece('b', 0);
	board[0][6].setPiece('n', 0);
	board[0][7].setPiece('r', 0);
	for(int i = 0; i < 8; i++)
        {
                board[6][i].setPiece('p', 1);
        }
	board[7][0].setPiece('r', 1);
        board[7][1].setPiece('n', 1);
        board[7][2].setPiece('b', 1);
        board[7][3].setPiece('q', 1);
        board[7][4].setPiece('k', 1);
        board[7][5].setPiece('b', 1);
        board[7][6].setPiece('n', 1);
        board[7][7].setPiece('r', 1);
	for(int i = 2; i < 6; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			board[i][j].deletePiece();
		}
	}

};

void Chess::Help()
{
	std::string in;
	thoughts << std::endl << "[1] How do I input my moves?\n[2] How do I win?\n[3] How do I see the board?\n[4] The computer made a mistake\n[5] Show commands\n";
	std::cin >> in;
	if(in == "1")
	{
		thoughts << "Pawn is denoted as 'p', knight as 'n', queen as 'q', rook as 'r', bishop as 'b', and king as 'k'\nRows are defined 1 through 8, and columns are defined a through h\n";
		thoughts << "To move a piece, type the first letter of the piece and the square.\nEx. Bishop to c5 ('bc5' is entered)\nDO NOT PUT 'p' IN FRONT OF PAWN MOVES\nEx. Pawn to e5 ('e5' is entered)\n";
		thoughts << "To capture with a pawn, type the column the pawn is on, then 'x', then the landing square.\nEx. The c-pawn captures d4 ('cxd4' is entered)\nDO NOT ADD 'x' WHEN CAPTURING WITH PIECES THAT AREN'T PAWNS\n";
		thoughts << "Ex. Bishop takes c5 ('bc5' is entered)\nIf you have two identical pieces that can move to the same square, specify which one with the column or row (whichever is different)\n";
		thoughts << "Ex. Both Knights can land on e4, one knight is on the g column, one is on the c column. You can move the g knight to e4 ('nge4' is entered)\nTo castle king's side, enter '0-0'\n";
		thoughts << "To castle queen's side, enter '0-0-0'" << std::endl;
	}
	else if (in == "2")
	{
		thoughts << "It's just chess rules, look it up " << std::endl;
	}
	else if (in == "3")
	{
		thoughts << "Enter 'board' to see the board.\n";
	}
	else if(in == "4")
	{
		thoughts << "Sorry bout' that" << std::endl;
	}
	else if(in == "5")
	{
		thoughts << "Enter 'undo' to undo your previous move\nEnter 'scores' to see how the bot scored each of its moves\n";
		thoughts << "Enter 'board' to see the board.\nEnter 'log' to see the list of played moves\nEnter 'stats' to see the status of each square on the board\nEnter 'resign' to end the game\nEnter 'quit' to end the game without";
		thoughts << " the computer bragging that it won\nEnter 'help' for the help menu\n";
	}
	else
	{
		thoughts << "Exiting help menu" << std::endl;
	}
}

void Chess::UndoMove()
{
	if(log.size() < 2)
	{
		thoughts << "You have not made a move yet" << std::endl;
		return;
	}
	for(int i = 0; i < 2; i++)
	{
		Move m = log.at(log.size()-1);
		if(!m.getPromotion())
			board[m.getStart().getRowIndex()][m.getStart().getColumnIndex()].setPiece(m.getEnd().getPiece(), m.getEnd().getColor());
		else
			board[m.getStart().getRowIndex()][m.getStart().getColumnIndex()].setPiece('p', m.getColor());
		if(m.getCapture() == ' ' || m.getEnPassant())
		{
			board[m.getEnd().getRowIndex()][m.getEnd().getColumnIndex()].deletePiece();
		}
		else
		{
			if(m.getColor() == 1)
				board[m.getEnd().getRowIndex()][m.getEnd().getColumnIndex()].setPiece(m.getCapture(), 0);
			else
				board[m.getEnd().getRowIndex()][m.getEnd().getColumnIndex()].setPiece(m.getCapture(), 1);
		}
		if(m.getKingsCastle())
		{
			if(m.getColor() == 0)
			{
				board[0][5].deletePiece();
				board[0][7].setPiece('r', 0);
				cancastlekingside = true;
			}
			else
			{
				board[7][5].deletePiece();
				board[7][7].setPiece('r', 1);
				canblackcastlekingside = true;
				if(veryspecificexception)
					canblackcastlequeenside = true;
			}
		}
		else if(m.getQueensCastle())
		{
			if(m.getColor() == 0)
			{
				board[0][3].deletePiece();
				board[0][0].setPiece('r', 0);
				cancastlequeenside = true;
			}
			else
			{
				board[7][3].deletePiece();
				board[7][0].setPiece('r', 1);
				if(veryspecificexception)
					canblackcastlekingside = true;
				canblackcastlequeenside = true;
			}
		}
		else if(m.getEnPassant())
		{
			if(m.getColor() == 0)
			{
				board[m.getEnd().getRowIndex()-1][m.getEnd().getColumnIndex()].setPiece('p', 1);
			}
			else
			{
				board[m.getEnd().getRowIndex()-1][m.getEnd().getColumnIndex()].setPiece('p', 0);
			}
		}
		log.pop_back();
	}
	turn--;
	thoughts << "Move undone." << std::endl;
}

bool Chess::BlacksMove(std::string move)
{
	std::string square;
	char piece;
	std::string tempmove = "";
	char file = ' ';
	if(move == "quit" || move == "stop" || move == "board" ||  move == "stats" || move == "resign" || move == "help" || move == "undo" || move == "log" || move == "why" || move == "scores")
	{
		return false;
	}
	for(unsigned int i = 0; i < move.length(); i++)
	{
		if(move.at(i) == '/')
		{
			forcemove = move.substr(i+1);
			tempmove = move.substr(0, i);
			move = tempmove;
		}
	}
	if(move.length() == 2)
	{
		piece = 'p';
		square = move;
	}
	else if(move.length() == 3)
	{
		if(move == "0-0")
		{
			piece = 'k';
			square = "g8";
		}
		else
		{
			piece = move.at(0);
			square = move.substr(1, 2);
		}
	}
	else if(move.length() == 4)
	{
		if(move.at(1) == 'x')
		{
			piece = 'p';
			file = move.at(0);
			square = move.substr(2, 2);
		}
		else if(move == "0-0-0")
		{
			piece = 'k';
			square = "c8";
		}
		else
		{
			piece = move.at(0);
			file = move.at(1);
			square = move.substr(2,2);
		}
	}
	else if(move == "0-0-0")
        {
                piece = 'k';
                square = "c8";
        }
	else
	{
		thoughts << "Not a valid move" << std::endl;
		return false;
	}

	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(board[i][j].SquareName() == square && board[i][j].getColor() != 1)
			{
				if(ValidMove(i, j, piece, 1, file))
				{
					return true;
				}
				else
				{
					if(blackcheck)
					{
						DefenseReset();
						if(blackcheck)
						{
							thoughts<< "You are in check!!" << std::endl;
						}
						else
						{
							thoughts << "That move would put you in check!" << std::endl;
						}
					}
					else
						thoughts << "That's an illegal move!" << std::endl;
					return false;

				}
			}
			else if(board[i][j].SquareName() == square)
			{
				thoughts << "You have a piece on " << square << "!" << std::endl;
				return false;
			}
		}

	}
	thoughts << piece << " to " << square << "? I don't understand that move" << std::endl;
	return false;

}

void Chess::TriggerBlackCheck()
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(board[i][j].getPiece() == 'k' && board[i][j].getColor() == 1 && board[i][j].getDefense() > 0)
			{
				blackcheck = true;
				return;
			}
		}
	}
	blackcheck = false;
}

void Chess::CheckForDraws()
{
	int blackmaterial = 0;
	int whitematerial = 0;
	int counter = 0;
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(board[i][j].getColor() == 0)
			{
				if(board[i][j].getPiece() == 'q' || board[i][j].getPiece() == 'p')
				{
					whitematerial += 9;
				}
				else if(board[i][j].getPiece() == 'r')
				{
					whitematerial += 5;
				}
				else if(board[i][j].getPiece() == 'b')
				{
					whitematerial += 3;
				}
				else if(board[i][j].getPiece() == 'n')
				{
					whitematerial += 2;
				}

			}
			else if(board[i][j].getColor() == 1)
			{
				if(board[i][j].getPiece() == 'q' || board[i][j].getPiece() == 'p')
                                {
                                        blackmaterial += 9;
                                }
                                else if(board[i][j].getPiece() == 'r')
                                {
                                        blackmaterial += 5;
                                }
                                else if(board[i][j].getPiece() == 'b')
                                {
                                        blackmaterial += 3;
                                }
                                else if(board[i][j].getPiece() == 'n')
                                {
                                        blackmaterial += 2;
                                }

			}
		}
	}
	if(blackmaterial < 5 && whitematerial < 5)
	{
		lackmaterial = true;
	}
	for(unsigned int i = 0; i < log.size(); i++)
	{
		if(log.at(i).getCapture() != ' ' || log.at(i).getPiece() == 'p')
		{
			counter = 0;
		}
		else
		{
			counter++;
		}

	}
	if(counter >= 50)
	{
		fiftymoverule = true;
	}
	for(unsigned int i = 5; i < log.size(); i++)
	{
		if(log.at(i) == log.at(i-2) && log.at(i-2) == log.at(i-4) && log.at(i-1) == log.at(i-3) && log.at(i-3) == log.at(i-5))
		{
			repetition = true;
		}
	}
}

void Chess::PromotionZone()
{
	std::string in = "";
	for(int i = 0; i < 8; i++)
	{
		if(board[0][i].getPiece() == 'p')
		{
			log.at(log.size()-1).setPromotion(true);
			while(in != "1" && in != "2" && in != "3" && in != "4")
			{
				thoughts << "Promote to:\n[1] Queen\n[2] Rook\n[3] Knight\n[4] Bishop\nchoice: ";
				std::cin >> in;
				if(in == "1")
				{
					board[0][i].setPiece('q', 1);
					log.at(log.size()-1).setPromotionPiece('q');
				}
				else if(in == "2")
				{
					board[0][i].setPiece('r', 1);
					log.at(log.size()-1).setPromotionPiece('r');
				}
				else if(in == "3")
				{
					board[0][i].setPiece('n', 1);
					log.at(log.size()-1).setPromotionPiece('n');
				}
				else if(in == "4")
				{
					board[0][i].setPiece('b', 1);
					log.at(log.size()-1).setPromotionPiece('b');
				}
				else
				{
					thoughts << "Invalid choice" << std::endl;
				}
			}
		}
		else if(board[7][i].getPiece() == 'p')
		{
			log.at(log.size()-1).setPromotion(true);
			thoughts << "I promote to a queen!" <<std::endl;
			board[7][i].setPiece('q', 0);
			log.at(log.size()-1).setPromotionPiece('q');
		}
	}
}

bool Chess::ValidMove(int row, int col, char piece, int color, char file, bool makemove)
{
	std::vector<Square> moveable;
	int filenum = -1;
	int rownum = -1;
	if(file != ' ')
	{
		if(file == 'a')
			filenum = 0;
		else if(file == 'b')
			filenum = 1;
		else if(file == 'c')
			filenum = 2;
		else if(file == 'd')
			filenum = 3;
		else if(file == 'e')
			filenum = 4;
		else if(file == 'f')
			filenum = 5;
		else if(file == 'g')
			filenum = 6;
		else if(file == 'h')
			filenum = 7;
		else if(file == '1' || file == '2' || file == '3' || file == '4' || file == '5' || file == '6' || file == '7' || file == '8')
		{
			rownum = (file - '0') - 1;
			file = ' ';
		}
		else
			return false;

	}
	if(row > 7 || row < 0 || col > 7 || col < 0)
	{
		return false;
	}
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(board[i][j].getPiece() == piece && board[i][j].getColor() == color)
			{
				moveable.push_back(board[i][j]);
			}
		}
	}
	for(Square s : moveable)
	{
		if(piece == 'p')
		{
			int direction = 1;
			if(color == 1)
			{
				direction = -1;
			}
			if(s.getRowIndex() + direction == row && s.getColumnIndex() == col && board[row][col].IsEmpty())
			{
				Square scopy = board[row][col];
				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
				board[row][col].setPiece('p', color);
				DefenseReset();
				if((blackcheck && color == 1) || (check && color == 0) || !makemove)
				{
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', color);
					board[row][col] = scopy;
					if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
				}
				Move move(turn, color, 'p', s, board[row][col]);
				log.push_back(move);
				return true;
			}
			else if(s.getRowIndex() == (int)(3.5 + direction*(-2.5)) && row == s.getRowIndex() + 2*direction && s.getColumnIndex() == col && board[row][col].IsEmpty() && board[(int)(3.5+(-1.5)*direction)][col].IsEmpty())
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('p', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', color);
                                        board[row][col] = scopy;
					if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				enpassant = true;
				enpassantcol = s.getColumnIndex();
				Move move(turn, color, 'p', s, board[row][col]);
                                log.push_back(move);
				return true;
			}
			else if((s.getRowIndex() + direction == row) && (s.getColumnIndex() + 1 == col || s.getColumnIndex() - 1 == col) && (board[row][col].getColor() != color) && (!board[row][col].IsEmpty()) && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('p', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', color);
                                        board[row][col] = scopy;
					if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'p', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;
			}
			else if(((color == 1 && s.getRowIndex() == 3 && row == 2 && board[row][col].IsEmpty()) || (color == 0 && s.getRowIndex() == 4 && row == 5 && board[row][col].IsEmpty())) && enpassant && col == enpassantcol)
			{
				Square scopy = board[s.getRowIndex()][col];
				board[row][col].setPiece('p', color);
				board[s.getRowIndex()][col].deletePiece();
				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
				DefenseReset();
				if((blackcheck && color == 1) || (check && color == 0) || !makemove)
				{
					board[row][col].deletePiece();
					board[s.getRowIndex()][col] = scopy;
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', color);
					if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
				}
				Move move(turn, color, 'p', s, board[row][col], scopy.getPiece(), false, false, true);
                                log.push_back(move);
				return true;

			}

		}
		if(piece == 'k')
		{
			if((s.getRowIndex() - 1 == row || s.getRowIndex() + 1 == row) && (s.getColumnIndex() - 1 == col || s.getColumnIndex() + 1 == col))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('k', color);
                                DefenseReset();
                                if((blackcheck && color ==1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', color);
                                        board[row][col] = scopy;
					if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				if(color == 1)
				{
					canblackcastlekingside = false;
                                        canblackcastlequeenside = false;
				}
				else
				{
					cancastlekingside = false;
					cancastlequeenside = false;
				}
				Move move(turn, color, 'k', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
				return true;
			}
			else if(s.getRowIndex() == row && (s.getColumnIndex() - 1 == col || s.getColumnIndex() + 1 == col))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('k', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', color);
                                        board[row][col] = scopy;
					if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				if(color == 1)
                                {
                                        canblackcastlekingside = false;
                                        canblackcastlequeenside = false;
                                }
				else
                                {
                                        cancastlekingside = false;
                                        cancastlequeenside = false;
                                }
				Move move(turn, color, 'k', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;

			}
			else if(s.getColumnIndex() == col && (s.getRowIndex() - 1 == row || s.getRowIndex() + 1 == row))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('k', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', color);
                                        board[row][col] = scopy;
					if((blackcheck && color == 1) || (check && color == 0))
                                        	return false;
					else
						return true;
                                }
				if(color == 1)
                                {
                                        canblackcastlekingside = false;
                                        canblackcastlequeenside = false;
                                }
				else
                                {
                                        cancastlekingside = false;
                                        cancastlequeenside = false;
                                }
				Move move(turn, color, 'k', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;

			}
			else if(s.SquareName() == "e8" && row == 7 && col == 6)
			{
				if(canblackcastlekingside && board[7][5].IsEmpty() && board[7][6].IsEmpty() && board[7][5].getDefense() == 0 && board[7][6].getDefense() == 0 && board[7][7].getPiece() == 'r' && board[7][7].getColor() == color)
				{
					if(!makemove)
						return true;
					board[7][6].setPiece('k', color);
					board[7][5].setPiece('r', color);
					board[7][4].deletePiece();
					board[7][7].deletePiece();
					if(canblackcastlequeenside)
						veryspecificexception = true;
					canblackcastlekingside = false;
					canblackcastlequeenside = false;
					Move move(turn, color, 'k', s, board[row][col], ' ', true);
                                	log.push_back(move);
					return true;
				}
			}
			else if(s.SquareName() == "e8" && row == 7 && col == 2)
			{
				if(canblackcastlequeenside && board[7][3].IsEmpty() && board[7][2].IsEmpty() && board[7][1].IsEmpty() && board[7][3].getDefense() == 0 && board[7][2].getDefense() == 0 && board[7][0].getPiece() == 'r' && board[7][0].getColor() == color)
				{
					if(!makemove)
						return true;
					board[7][2].setPiece('k', color);
					board[7][3].setPiece('r', color);
					board[7][4].deletePiece();
					if(canblackcastlekingside)
						veryspecificexception = true;
					board[7][0].deletePiece();
					canblackcastlekingside = false;
					canblackcastlequeenside = false;
					Move move(turn, color, 'k', s, board[row][col], ' ', false, true);
                                	log.push_back(move);
					return true;
				}
			}

		}
		if(piece == 'n')
		{
			if(s.getColumnIndex() - 2 == col && (s.getRowIndex() - 1 == row || s.getRowIndex() + 1 == row) && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('n', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('n', color);
                                        board[row][col] = scopy;
                                        if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'n', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;
			}
			else if(s.getColumnIndex() + 2 == col && (s.getRowIndex() - 1 == row || s.getRowIndex() + 1 == row) && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('n', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('n', color);
                                        board[row][col] = scopy;
                                        if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'n', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;
			}
			else if(s.getRowIndex() - 2 == row && (s.getColumnIndex() - 1 == col || s.getColumnIndex() + 1 == col) && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('n', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('n', color);
                                        board[row][col] = scopy;
                                        if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'n', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;

			}
			else if(s.getRowIndex() + 2 == row && (s.getColumnIndex() - 1 == col || s.getColumnIndex() + 1 == col) && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
			{
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('n', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('n', color);
                                        board[row][col] = scopy;
                                        if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'n', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;

			}


		}
		if(piece == 'b')
		{
			int x = row - s.getRowIndex();
			int y = col - s.getColumnIndex();
			if(x*x == y*y && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
			{
				for(int i = 1; i < std::abs(x); i++)
				{
					if(x > 0 && y > 0)
					{
						if(!board[s.getRowIndex()+i][s.getColumnIndex()+i].IsEmpty())
						{
							return false;
						}
					}
					if(x > 0 && y < 0)
					{
						if(!board[s.getRowIndex()+i][s.getColumnIndex()-i].IsEmpty())
                                                {
                                                        return false;
                                                }
	
					}
					if(x < 0 && y > 0)
					{
						if(!board[s.getRowIndex()-i][s.getColumnIndex()+i].IsEmpty())
                                                {
                                                        return false;
                                                }

					}
					if(x < 0 && y < 0)
					{
						if(!board[s.getRowIndex()-i][s.getColumnIndex()-i].IsEmpty())
                                                {
                                                        return false;
                                                }

					}
				}
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('b', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', color);
                                        board[row][col] = scopy;
                                        if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'b', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;
			}
 	
		}
		if(piece == 'q')
		{
			int x = row - s.getRowIndex();
                        int y = col - s.getColumnIndex();
                        if(x*x == y*y && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
                        {
				for(int i = 1; i < std::abs(x); i++)
                                {
                                        if(x > 0 && y > 0)
                                        {
                                                if(!board[s.getRowIndex()+i][s.getColumnIndex()+i].IsEmpty())
                                                {
                                                        return false;
                                                }
                                        }
                                        if(x > 0 && y < 0)
                                        {
                                                if(!board[s.getRowIndex()+i][s.getColumnIndex()-i].IsEmpty())
                                                {
                                                        return false;
                                                }

                                        }
                                        if(x < 0 && y > 0)
                                        {
                                                if(!board[s.getRowIndex()-i][s.getColumnIndex()+i].IsEmpty())
                                                {
                                                        return false;
                                                }

                                        }
                                        if(x < 0 && y < 0)
                                        {
                                                if(!board[s.getRowIndex()-i][s.getColumnIndex()-i].IsEmpty())
                                                {
                                                        return false;
                                                }

                                        }
                                }

				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('q', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', color);
                                        board[row][col] = scopy;
                                        if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'q', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;
                        }

			if((s.getRowIndex() == row || s.getColumnIndex() == col) && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
                        {
				if(s.getRowIndex() == row && col - s.getColumnIndex() > 0)
				{
					for(int i = 1; i < std::abs(col - s.getColumnIndex()) ; i++)
                                	{
                                                	if(!board[s.getRowIndex()][s.getColumnIndex()+i].IsEmpty())
                                                	{
                                                        	return false;
                                                	}
                                	}
				}
				
				else if(s.getRowIndex() == row)
                                {
                                        for(int i = 1; i < std::abs(col - s.getColumnIndex()) ; i++)
                                        {
                                                        if(!board[s.getRowIndex()][s.getColumnIndex()-i].IsEmpty())
                                                        {
                                                                return false;
                                                        }
                                        }
                                }
				else if(row - s.getRowIndex() > 0)
				{
					for(int i = 1; i < std::abs(row - s.getRowIndex()) ; i++)
                                        {
                                                        if(!board[s.getRowIndex()+i][s.getColumnIndex()].IsEmpty())
                                                        {
                                                                return false;
                                                        }
                                        }

				}
				else
				{
					for(int i = 1; i < std::abs(row - s.getRowIndex()) ; i++)
                                        {
                                                        if(!board[s.getRowIndex()-i][s.getColumnIndex()].IsEmpty())
                                                        {
                                                                return false;
                                                        }
                                        }

				}
               	
				Square scopy = board[row][col];
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                board[row][col].setPiece('q', color);
                                DefenseReset();
                                if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                {
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', color);
                                        board[row][col] = scopy;
                                        if((blackcheck && color == 1) || (check && color == 0))
                                                return false;
                                        else
                                                return true;
                                }
				Move move(turn, color, 'q', s, board[row][col], scopy.getPiece());
                                log.push_back(move);
                                return true;

                        }

		}
		if(piece == 'r')
		{
			bool skip = false;
			if((s.getRowIndex() == row || s.getColumnIndex() == col) && (s.getColumnIndex() == filenum || file == ' ') && (rownum == -1 || s.getRowIndex() == rownum))
			{
				if(s.getRowIndex() == row && col - s.getColumnIndex() > 0)
                                {
                                        for(int i = 1; i < std::abs(col - s.getColumnIndex()) ; i++)
                                        {
                                                        if(!board[s.getRowIndex()][s.getColumnIndex()+i].IsEmpty())
                                                        {
                                                                skip = true;
                                                        }
                                        }
                                }

                                else if(s.getRowIndex() == row)
                                {
                                        for(int i = 1; i < std::abs(col - s.getColumnIndex()) ; i++)
                                        {
                                                        if(!board[s.getRowIndex()][s.getColumnIndex()-i].IsEmpty())
                                                        {
                                                                skip = true;
                                                        }
                                        }
                                }
                                else if(row - s.getRowIndex() > 0)
                                {
                                        for(int i = 1; i < std::abs(row - s.getRowIndex()) ; i++)
                                        {
                                                        if(!board[s.getRowIndex()+i][s.getColumnIndex()].IsEmpty())
                                                        {
                                                                skip = true;
                                                        }
                                        }

                                }
                                else
                                {
                                        for(int i = 1; i < std::abs(row - s.getRowIndex()) ; i++)
                                        {
                                                        if(!board[s.getRowIndex()-i][s.getColumnIndex()].IsEmpty())
                                                        {
                                                                skip = true;
                                                        }
                                        }

                                }
				if(!skip)
				{
					Square scopy = board[row][col];
                                	board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                	board[row][col].setPiece('r', color);
                                	DefenseReset();
                                	if((blackcheck && color == 1) || (check && color == 0) || !makemove)
                                	{
                                        	board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', color);
                                        	board[row][col] = scopy;
                                        	if((blackcheck && color == 1) || (check && color == 0))
                                                	return false;
                                        	else
                                                	return true;
                                	}
					if(color == 1)
                                	{
						if(s.getRowIndex() == 7 && s.getColumnIndex() == 7)
                                        		canblackcastlekingside = false;
						else if(s.getRowIndex() == 7 && s.getColumnIndex() == 0)
                                        		canblackcastlequeenside = false;
                                	}
					if(color == 0)
                                        {
                                                if(s.getRowIndex() == 7 && s.getColumnIndex() == 7)
                                                        cancastlekingside = false;
                                                else if(s.getRowIndex() == 7 && s.getColumnIndex() == 0)
                                                        cancastlequeenside = false;
                                        }
					Move move(turn, color, 'r', s, board[row][col], scopy.getPiece());
                                	log.push_back(move);

                                	return true;
				}

			}
		}
	}
	return false;
}

void Chess::TriggerCheck()
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(board[i][j].getPiece() == 'k'  && board[i][j].getAttack() > 0 && board[i][j].getColor() == 0)
			{
				check = true;
				return;
			}
		}
	}
	check = false;
}

void Chess::DefenseReset()
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			board[i][j].resetDefense();
			board[i][j].resetAttacks();
		}
	}
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			for(int k = 0; k < 8; k++)
			{
				for(int l = 0; l < 8; l++)
				{
					if(k == i && l == j)
					{
					}
					else if(board[k][l].getPiece() == 'p')
					{
						if(board[k][l].getColor() == 0 && k + 1 == i && (l + 1 == j || l - 1 == j))
						{
								board[i][j].addDefender('p', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addDefense();
						}
						else if(board[k][l].getColor() == 1 && k - 1 == i && (l + 1 == j || l - 1 == j))
						{
								board[i][j].addAttacker('p', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addAttack();
						}
					}
					else if(board[k][l].getPiece() == 'n')
					{
						if(l - 2 == j && (k - 1 == i || k + 1 == i))
                        			{
                                			if(board[k][l].getColor() == 1)
							{
								board[i][j].addAttacker('n', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addAttack();
							}
							else
							{
								board[i][j].addDefender('n', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addDefense();
							}
		    				}
                        			else if(l + 2 == j && (k - 1 == i || k + 1 == i))
                        			{
							if(board[k][l].getColor() == 1)
                                                        {
								board[i][j].addAttacker('n', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addAttack();
                                                        }
                                                        else
                                                        {
								board[i][j].addDefender('n', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addDefense();
                                                        }

                                			
                        			}
                        			else if(k - 2 == i && (l - 1 == j || l + 1 == j))
                        			{
							if(board[k][l].getColor() == 1)
                                                        {
								board[i][j].addAttacker('n', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addAttack();
                                                        }
                                                        else
                                                        {
								board[i][j].addDefender('n', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addDefense();
                                                        }                                			

                        			}
                        			else if(k + 2 == i && (l - 1 == j || l + 1 == j))
                        			{
							if(board[k][l].getColor() == 1)
                                                        {
								board[i][j].addAttacker('n', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addAttack();
                                                        }
                                                        else
                                                        {
								board[i][j].addDefender('n', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addDefense();
                                                        }
                                			

                        			}


					}
					else if(board[k][l].getPiece() == 'k')
					{
						if((k - 1 == i || k + 1 == i) && (l - 1 == j || l + 1 == j))
                        			{
                                			if(board[k][l].getColor() == 1)
							{
								board[i][j].addAttacker('k', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addAttack();
							}
							else
							{
								board[i][j].addDefender('k', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addDefense();
							}
                        			}
                        			else if((k == i) && (l - 1 == j || l + 1 == j))
                        			{
                                			if(board[k][l].getColor() == 1)
                                                        {
								board[i][j].addAttacker('k', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addAttack();
                                                        }
                                                        else
                                                        {
								board[i][j].addDefender('k', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addDefense();
                                                        }


                        			}
                        			else if(l == j && (k - 1 == i || k + 1 == i))
                        			{
                                			if(board[k][l].getColor() == 1)
                                                        {
								board[i][j].addAttacker('k', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addAttack();
                                                        }
                                                        else
							{
								board[i][j].addDefender('k', std::stoi(std::to_string(k)+std::to_string(l)));
								board[i][j].addDefense();
                                                        }


                        			}

					}
					else if(board[k][l].getPiece() == 'q')
					{
						bool blocked = false;
						int x = i - k;
                        			int y = j - l;
                        			if(x*x == y*y)
                        			{
							if(x*x != 1)
							{
								if(x > 0 && y > 0)
								{
									for(int p = 1; p < std::abs(x); p++)
									{
										if(!board[k+p][l+p].IsEmpty())
										{
											blocked = true;
										}
									}
								}
								else if (x > 0 && y < 0)
								{
									for(int p = 1; p < std::abs(x); p++)
									{
										if(!board[k+p][l-p].IsEmpty())
										{
											blocked = true;
										}
									}
								}
								else if(x < 0 && y > 0)
								{
									for(int p = 1; p < std::abs(x); p++)
                                                                        {
                                                                                if(!board[k-p][l+p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }

								}
								else if(x < 0 && y < 0)
								{
									for(int p = 1; p < std::abs(x); p++)
                                                                        {
                                                                                if(!board[k-p][l-p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }

								}
							}
                                			if(board[k][l].getColor() == 1 && !blocked)
                                                        {
								board[i][j].addAttacker('q', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addAttack();
                                                        }
                                                        else if(!blocked)
                                                        {
								board[i][j].addDefender('q', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addDefense();
                                                        }

                        			}

						else if(k == i || l == j)
                        			{
							if(std::abs(k - i) > 1 || std::abs(l - j) > 1)
                                                        {
                                                                if(k - i > 0 && l - j == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(k - i); p++)
                                                                        {
                                                                                if(!board[k-p][l].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }
                                                                }
                                                                else if (k - i < 0 && l - j == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(k-i); p++)
                                                                        {
                                                                                if(!board[k+p][l].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }
                                                                }
                                                                else if(l - j > 0 && k - i == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(l-j); p++)
                                                                        {
                                                                                if(!board[k][l-p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                } 
                                                                        }

                                                                }
                                                                else if(l - j < 0 && k - i == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(l-j); p++)
                                                                        {
                                                                                if(!board[k][l+p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }

                                                                }
							}

                                			if(board[k][l].getColor() == 1 && !blocked)
                                                        {
								board[i][j].addAttacker('q', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addAttack();
                                                        }
                                                        else if(!blocked)
                                                        {
								board[i][j].addDefender('q', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                board[i][j].addDefense();
                                                        }

                        			}




					}
					else if(board[k][l].getPiece() == 'b')
					{
						bool blocked = false;
						int x = i - k;
                        			int y = j - l;
                        			if(x*x == y*y)
                        			{
							if(x*x != 1)
                                                        {
                                                                if(x > 0 && y > 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(x); p++)
                                                                        {
                                                                                if(!board[k+p][l+p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }
                                                                }
                                                                else if (x > 0 && y < 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(x); p++)
                                                                        {
                                                                                if(!board[k+p][l-p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }
                                                                }
                                                                else if(x < 0 && y > 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(x); p++)
                                                                        {
                                                                                if(!board[k-p][l+p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }

                                                                }
                                                                else if(x < 0 && y < 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(x); p++)
                                                                        {
                                                                                if(!board[k-p][l-p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }

                                                                }
							}

                                			  if(board[k][l].getColor() == 1 && !blocked)
                                                          {
								  board[i][j].addAttacker('b', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                  board[i][j].addAttack();
                                                          }
                                                          else if(!blocked)
                                                          {
								  board[i][j].addDefender('b', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                  board[i][j].addDefense();
                                                          }

                        			}

					}
					else if(board[k][l].getPiece() == 'r')
					{
						bool blocked = false;
						if(k == i || l == j)
                        			{
							if(std::abs(k - i) > 1 || std::abs(l - j) > 1)
                                                        {
                                                                if(k - i > 0 && l - j == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(k - i); p++)
                                                                        {
                                                                                if(!board[k-p][l].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }
                                                                }
                                                                else if (k - i < 0 && l - j == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(k-i); p++)
                                                                        {
                                                                                if(!board[k+p][l].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }
                                                                }
                                                                else if(l - j > 0 && k - i == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(l-j); p++)
                                                                        {
                                                                                if(!board[k][l-p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }

                                                                }
                                                                else if(l - j < 0 && k - i == 0)
                                                                {
                                                                        for(int p = 1; p < std::abs(l-j); p++)
                                                                        {
                                                                                if(!board[k][l+p].IsEmpty())
                                                                                {
                                                                                        blocked = true;
                                                                                }
                                                                        }

                                                                }
                                                        }

                                			    if(board[k][l].getColor() == 1  && !blocked)
                                                            {
								    board[i][j].addAttacker('r', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                    board[i][j].addAttack();
                                                            }
                                                            else if(!blocked)
                                                            {
								    board[i][j].addDefender('r', std::stoi(std::to_string(k)+std::to_string(l)));
                                                                    board[i][j].addDefense();
                                                            }


                        			}

					}
				}
			}
		}
	}
	TriggerCheck();
	TriggerBlackCheck();
}

void Chess::PrintSquares()
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			std::cout << board[i][j].SquareName() << " Defense Number: " << board[i][j].getDefense() << " {";
		        for(char c : board[i][j].getDefenders())
			{
				std::cout << c << ", ";
			}	
			std::cout << "} Attack Number: " << board[i][j].getAttack() << " {";
		        for(char c : board[i][j].getAttackers())
			{
				std::cout << c << ", ";
			}
			std::cout << "} Piece: ";
			if(board[i][j].getColor() == 0)
			{
				std::cout << "White ";
			}
			else if(board[i][j].getColor() == 1)
			{
				std::cout << "Black ";
			}
			if(board[i][j].IsEmpty())
			{
				std::cout << "n/a" << std::endl;
			}
			else
			{
				std::cout << board[i][j].getPiece() << std::endl;
			}
		}
	}
}

int Chess::CalcScore(Move move)
{
	int score = 0;
	bool toggle = false;
	bool toggle2 = false;
	int beforedefense[8][8];
	int attackedpieces = 0;
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			beforedefense[i][j] = board[i][j].getDefense();
		}
	}
	board[move.getStart().getRowIndex()][move.getStart().getColumnIndex()].deletePiece();
	Square scopy = board[move.getEnd().getRowIndex()][move.getEnd().getColumnIndex()]; 
	scores << "--Looking at " << move.getPiece() << ": " << move.getStart().SquareName();
	if(move.getCapture() != ' ')
	{
        	scores << " takes ";
	}
	else
		scores << " to ";
	scores << move.getEnd().SquareName() << "\n---------------------------------------------------" << std::endl;
	if(!move.getPromotion())
		board[move.getEnd().getRowIndex()][move.getEnd().getColumnIndex()].setPiece(move.getPiece(), move.getColor());
	if(move.getEnPassant())
	{
		board[4][enpassantcol].deletePiece();
	}
	if(move.getKingsCastle())
	{
		board[0][7].deletePiece();
		board[0][5].setPiece('r', move.getColor());
	}
	if(move.getQueensCastle())
	{
		board[0][0].deletePiece();
		board[0][3].setPiece('r', move.getColor());
	}
	if(move.getPromotion())
	{
		board[7][move.getEnd().getColumnIndex()].setPiece('q', 0);
	}
	DefenseReset();
	if(DidIWin())
	{
		scores << "There is mate in 1 (+999999999)" << std::endl;
		score += 999999999;
	}
	else if(DidIWin(true))
	{
		scores << "This would hang mate in 1 (-99999999)" << std::endl;
		score -= 99999999;
	}
	// INCREASES TIME SIGNIFICANTLY
	/*else if(DidIWin(false, true))
	{
		scores << "There is mate in 2 (+99999999)" << std::endl;
		score += 99999999;
	}
	else if(DidIWin(false, false, true))
	{
		scores << "This would hang mate in 2 (-9999999)" << std::endl;
		score -= 9999999;
	}*/
	if(move.getCapture() == 'q' && move.getPiece() != 'q')
	{
		scores << "This would win a queen (+99999)" << std::endl;
		score += 99999;
	}
	if(move.getPiece() == 'p' && move.getEnd().getRowIndex() == 7)
	{
		if(move.getEnd().getAttack() == 0)
		{
			scores << "This would promote a pawn with no contest (+99999)" << std::endl;
			score += 99999;
		}
		else
		{
			scores << "This would promote a pawn, although it might be taken (+50)" << std::endl;
			score+=50;
		}
	}
	if(move.getCapture() == 'q' && move.getPiece() == 'q')
	{
		scores << "This would trade queens (+50)" << std::endl;
		score += 100;
	}
	if(move.getCapture() != ' ' && move.getCapture() != 'p' && move.getPiece() == 'p')
	{
		scores << "This captures a hanging piece with my pawn! (+99999)" << std::endl;
		score += 99999;
	}
	if(move.getCapture() == 'p' && move.getPiece() == 'p')
	{
		if(move.getEnd().getAttack() == 0)
		{
			scores << "This simply wins a pawn (+300)" << std::endl;
			score +=300;
		}
		else if(log.at(log.size()-1).getCapture() != 'p')
		{
			scores << "This would trade pawn (+100)" << std::endl;
			score += 100;
		}
		else
		{
			scores << "My opponent took a pawn, so i'll take a pawn (+175)" << std::endl;
			score+=175;
		}
	}
	if(move.getCapture() != ' ' && move.getEnd().getAttack() == 0)
	{
		scores << "This is free material (+200)" << std::endl;
		score+=200;
	}
	if(move.getCapture() == 'r' && (move.getPiece() == 'b' || move.getPiece() == 'n'))
	{
		scores << "This would capture a rook and put me up an exchange! (+99999)" << std::endl;
		score += 99999;
	}
	if(move.getCapture() != 'q' && move.getCapture() != ' ' && move.getPiece() == 'q' && move.getEnd().getAttack() > 0)
	{
		for(char c : move.getEnd().getAttackers())
                {
                        if(c != 'q')
                        {
                                scores << "I am using my queen to capture a lesser defended piece! (-999)" << std::endl;
                                score -=999;
                                toggle2 = true;
                        }
                }
                if(!toggle2 && move.getEnd().getAttack() < move.getEnd().getDefense())
                {
                        scores << "I will win a piece after trades are done (+999)" << std::endl;
                        score += 999;
                }
	}
	toggle2 = false;
	if(move.getCapture() != 'q' && move.getCapture() != ' ' && move.getCapture() != 'r' && move.getPiece() == 'r' && move.getEnd().getAttack() > 0)
	{
		for(char c : move.getEnd().getAttackers())
		{
			if(c != 'q' && c != 'r')
			{
				scores << "I am using my rook to capture a lesser defended piece! (-999)" << std::endl;
                		score -=999;
				toggle2 = true;
			}
		}
		if(!toggle2 && move.getEnd().getAttack() < move.getEnd().getDefense())
		{
			scores << "I will win a piece after trades are done (+999)" << std::endl;
			score += 999;
		}
	}
	DefenseReset();
	if(blackcheck)
	{
		scores << "This would be check! (+50)" << std::endl;
		score+=50;
	}
	int backupattackers = 0;
	int backupdefenders = 0;
	int row = 0;
	int col = 0;
	for(int i = 0; i < 8; i++)
        {
              for(int j = 0; j < 8; j++)
              {
		      backupattackers = 0;
		      backupdefenders = 0;
              	if(board[i][j].getColor() == 0  && board[i][j].getAttack() > 0)
                {
			for(unsigned int k = 0; k < board[i][j].getAttackerSquares().size(); k++)
			{
				row = board[i][j].getAttackerSquares().at(k) / 10;
				col = board[i][j].getAttackerSquares().at(k) % 10;
				if(board[i][j].getAttackers().at(k) == 'b' || board[i][j].getAttackers().at(k) == 'q')
				{
					if(i > row && j < col)
					{
						for(int l = 1; l < 8; l++)
						{
							if(row-l >= 0 && col+l < 8 && board[row-l][col+l].getColor() == 1 && (board[row-l][col+l].getPiece() == 'q' || board[row-l][col+l].getPiece() == 'b'))
							{
								backupattackers++;
							}	
						}
					}
					else if(i > row && j > col)
					{
						for(int l = 1; l < 8; l++)
                                                {
                                                        if(row-l >= 0 && col-l >= 0 && board[row-l][col-l].getColor() == 1 && (board[row-l][col-l].getPiece() == 'q' || board[row-l][col-l].getPiece() == 'b'))
                                                        {
                                                                backupattackers++;
                                                        }
                                                }

					}
					else if(i < row && j < col)
					{
						for(int l = 1; l < 8; l++)
                                                {
                                                        if(row+l < 8 && col+l < 8 && board[row+l][col+l].getColor() == 1 && (board[row+l][col+l].getPiece() == 'q' || board[row+l][col+l].getPiece() == 'b'))
                                                        {
                                                                backupattackers++;
                                                        }
                                                }
					}
					else if(i < row && j < col)
					{
						for(int l = 1; l < 8; l++)
                                                {
                                                        if(row+l < 8 && col-l >= 0 && board[row+l][col-l].getColor() == 1 && (board[row+l][col-l].getPiece() == 'q' || board[row+l][col-l].getPiece() == 'b'))
                                                        {
                                                                backupattackers++;
                                                        }
                                                }
					}
				}
				if(board[i][j].getAttackers().at(k) == 'r' || board[i][j].getAttackers().at(k) == 'q')
                                {
                                        if(i == row  && j > col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(col-l >= 0 && board[row][col-l].getColor() == 1 && (board[row][col-l].getPiece() == 'q' || board[row][col-l].getPiece() == 'r'))
                                                        {
                                                                backupattackers++;
                                                        }
                                                }
                                        }
                                        else if(i == row && j < col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(col+l < 8 && board[row][col+l].getColor() == 1 && (board[row][col+l].getPiece() == 'q' || board[row][col+l].getPiece() == 'r'))
                                                        {
                                                                backupattackers++;
                                                        }
                                                }

                                        }
                                        else if(i < row && j == col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row+l < 8 && board[row+l][col].getColor() == 1 && (board[row+l][col].getPiece() == 'q' || board[row+l][col].getPiece() == 'r'))
                                                        {
                                                                backupattackers++;
                                                        }
                                                }
                                        }
                                        else if(i > row && j == col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row-l >= 0 && board[row-l][col].getColor() == 1 && (board[row-l][col].getPiece() == 'q' || board[row-l][col].getPiece() == 'r'))
                                                        {
                                                                backupattackers++;
                                                        }
                                                }
                                        }
                                }
			}
			for(unsigned int k = 0; k < board[i][j].getDefenderSquares().size(); k++)
                        {
                                row = board[i][j].getDefenderSquares().at(k) / 10;
                                col = board[i][j].getDefenderSquares().at(k) % 10;
                                if(board[i][j].getDefenders().at(k) == 'b' || board[i][j].getDefenders().at(k) == 'q')
                                {
                                        if(i > row && j < col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row-l >= 0 && col+l < 8 && board[row-l][col+l].getColor() == 0 && (board[row-l][col+l].getPiece() == 'q' || board[row-l][col+l].getPiece() == 'b'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }
                                        }
                                        else if(i > row && j > col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row-l >= 0 && col-l >= 0 && board[row-l][col-l].getColor() == 0 && (board[row-l][col-l].getPiece() == 'q' || board[row-l][col-l].getPiece() == 'b'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }

                                        }
                                        else if(i < row && j < col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row+l < 8 && col+l < 8 && board[row+l][col+l].getColor() == 0 && (board[row+l][col+l].getPiece() == 'q' || board[row+l][col+l].getPiece() == 'b'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }
                                        }
                                        else if(i < row && j < col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row+l < 8 && col-l >= 0 && board[row+l][col-l].getColor() == 0 && (board[row+l][col-l].getPiece() == 'q' || board[row+l][col-l].getPiece() == 'b'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }
                                        }
                                }
				if(board[i][j].getDefenders().at(k) == 'r' || board[i][j].getDefenders().at(k) == 'q')
                                {
                                        if(i == row  && j > col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(col-l >= 0 && board[row][col-l].getColor() == 0 && (board[row][col-l].getPiece() == 'q' || board[row][col-l].getPiece() == 'r'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }
                                        }
                                        else if(i == row && j < col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(col+l < 8 && board[row][col+l].getColor() == 0 && (board[row][col+l].getPiece() == 'q' || board[row][col+l].getPiece() == 'r'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }

                                        }
                                        else if(i < row && j == col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row+l < 8 && board[row+l][col].getColor() == 0 && (board[row+l][col].getPiece() == 'q' || board[row+l][col].getPiece() == 'r'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }
                                        }
                                        else if(i > row && j == col)
                                        {
                                                for(int l = 1; l < 8; l++)
                                                {
                                                        if(row-l >= 0 && board[row-l][col].getColor() == 0 && (board[row-l][col].getPiece() == 'q' || board[row-l][col].getPiece() == 'r'))
                                                        {
                                                                backupdefenders++;
                                                        }
                                                }
                                        }
                                }
			}

			        if(board[i][j].getPiece() == 'p')
				{
					for(char c : board[i][j].getDefenders())
					{
						if( c == 'p')
						{
							toggle = true;
							break;
						}
					}
					if(!toggle && board[i][j].getDefense() + backupdefenders < board[i][j].getAttack() + backupattackers)
					{
						if(move.getCapture() == ' ')
						{
							scores << "My pawn on " << board[i][j].SquareName() << " would hang and it isn't defended by a pawn! (-555)" << std::endl;
							score -= 555;
						}
					}
				}
				else if(board[i][j].getPiece() == 'q')
				{
					toggle = false;
					for(char c : board[i][j].getAttackers())
					{
						if(c != 'q')
						{
							toggle = true;
							if(move.getCapture() != 'q')
							{
								scores << "This would hang my queen on " << board[i][j].SquareName() << "! (-999999)" << std::endl; 
                                				score -= 999999;
							}
							break;
						}
						if(!toggle && board[i][j].getDefense() == 0)
						{
							if(move.getCapture() != 'q')
							{
								scores << "This would hang my queen on " << board[i][j].SquareName() << "! (-999999)" << std::endl;
								score -= 999999;
							}
						}

					}
				}
				else if(board[i][j].getPiece() == 'r')
				{
					toggle = false;
					for(char c : board[i][j].getAttackers())
					{
						if(c != 'r' && c != 'q')
						{
							toggle = true;
							if(move.getCapture() != 'r' && move.getCapture() != 'q')
							{
								scores << "This would hang my rook on " << board[i][j].SquareName() << "! (-555555)" << std::endl;
								score -= 555555;
							}
							break;
						}
						if(!toggle && board[i][j].getDefense() + backupdefenders < board[i][j].getAttack() + backupattackers)
						{
							if(move.getCapture() != 'r' && move.getCapture() != 'q')
							{
								scores << "This would hang my rook on " << board[i][j].SquareName() << "! (-555555)" << std::endl;
								score -= 555555;
							}
						}
					}
				}
				else if (board[i][j].getPiece() == 'n' || board[i][j].getPiece() == 'b')
				{
					toggle = false;
					for(char c : board[i][j].getAttackers())
					{
						if(c == 'p')
						{
							toggle = true;
							if(move.getCapture() == 'p' || move.getCapture() == ' ')
							{
								scores << "This would hang my bishop/knight on " << board[i][j].SquareName() << "! (-99999)" << std::endl;
								score -= 99999;
							}
							break;
						}
					}
					if(!toggle && board[i][j].getDefense() + backupdefenders < board[i][j].getAttack() + backupattackers)
					{
						if(move.getCapture() == 'p' || move.getCapture() == ' ')
						{
							scores << "This would hang my bishop/knight on " << board[i][j].SquareName() << "! (-99999)" << std::endl;
							score -= 99999;
						}
					}
				}
                }
		if(board[i][j].getColor() == 1 && board[i][j].getPiece() == 'q' && (board[i][j].getDefense() > beforedefense[i][j]))
		{
			scores << "This would threaten to win the enemy queen! (+75)" << std::endl;
			score +=75;
			attackedpieces++;
		}
		if(board[i][j].getColor() == 1 && board[i][j].getPiece() == 'r' && (board[i][j].getDefense() > beforedefense[i][j]) && (board[i][j].getAttack() < board[i][j].getDefense() || move.getPiece() != 'q'))
                {
                        scores << "This would threaten to win an enemy rook! (+75)" << std::endl;
                        score +=75;
			attackedpieces++;
                }
		if(board[i][j].getColor() == 1 && (board[i][j].getPiece() == 'b' || board[i][j].getPiece() == 'n') && (board[i][j].getDefense() > beforedefense[i][j]) && (board[i][j].getAttack() < board[i][j].getDefense() || (move.getPiece() == 'p')))
                {
                        scores << "This would threaten to win an enemy knight/bishop! (+75)" << std::endl;
                        score +=75;
			attackedpieces++;
                }
		if(board[i][j].getColor() == 1 && board[i][j].getPiece() == 'p' && (board[i][j].getDefense() > beforedefense[i][j]) && move.getEnd().getAttack() == 0)
                {
                        scores << "This would threaten to win a pawn! (+75)" << std::endl;
                        score +=75;
			attackedpieces++;
                }
		if(attackedpieces > 1)
		{
			scores << "This would be a fork! (+100)" << std::endl;
			score += 100;
		}
               }
	}
	if(move.getTurn() < 20 && (cancastlekingside || cancastlequeenside) && (move.getPiece() == 'r' || (move.getPiece() == 'k' && (!move.getKingsCastle() && !move.getQueensCastle()))))
	{
		scores << "This would unneccessarily jeopardize castling ability (-100)" << std::endl;
		score -= 100;
	}
	if(move.getTurn() < 20 && move.getPiece() == 'k' && move.getEnd().getRowIndex() > move.getStart().getRowIndex())
	{
		scores << "This would be moving my king towards the center (-100)" << std::endl;
		score-=100;
	}
	if(move.getKingsCastle() || move.getQueensCastle() || move.getEnPassant())
	{
		scores << "This is a special move! (+100)" << std::endl;
		score += 100;
	}
	if(move.getPiece() == 'p' && (move.getEnd().SquareName() == "e4" || move.getEnd().SquareName() == "d4" || move.getEnd().SquareName() == "d3" || move.getEnd().SquareName() == "e3") && move.getTurn() < 3)
	{
		if((move.getStart().SquareName() == "d3" || move.getStart().SquareName() == "e3") && move.getTurn() < 10)
		{
			scores << "I just pushed this pawn to row 3 (-50)" << std::endl;
			score-=50;
		}
		else
		{
			scores << "This is a solid opening move (+50)" << std::endl;
			score+=50;
		}
	}
	if(move.getPiece()=='n' && (move.getEnd().SquareName() == "c3" || move.getEnd().SquareName() == "f3") && move.getTurn() < 3)
	{
		scores << "This develops a knight early! (+50)" << std::endl;
		score+=50;
	}
	if(move.getPiece() == 'b' && (move.getStart().SquareName() == "f1" || move.getStart().SquareName() == "c1") && move.getTurn() < 10)
	{
		scores << "This develops my bishop! (+50)" << std::endl;
		score+=50;
	}
	if(move.getPiece() == 'p' && (move.getEnd().SquareName() == "g4" || move.getEnd().SquareName() == "g3" || move.getEnd().SquareName() == "h3" || move.getEnd().SquareName() == "h4" || move.getEnd().SquareName() == "a3" || move.getEnd().SquareName() == "a4" || move.getEnd().SquareName() == "b3" || move.getEnd().SquareName() == "b4") && move.getTurn() < 15)
	{
		scores << "This is an unreasonable pawn push (-50)" << std::endl;
		score -=50;
	}
	if(move.getPiece() == 'b' && (move.getStart().SquareName() != "f1" && move.getStart().SquareName() != "c1") && move.getTurn() < 6)
	{
		scores << "I'd be moving my bishop again! (-25)" << std::endl;
		score -=25;
	}
	if(move.getPiece() == 'b' && move.getEnd().SquareName() == "e3" && board[1][4].getPiece() == 'p')
        {
                scores << "I'd be moving my bishop in front of my e pawn (-40)" << std::endl;
                score -=40;
        }
	if(move.getPiece() == 'b' && move.getEnd().SquareName() == "d3" && board[1][3].getPiece() == 'p')
        {
                scores << "I'd be moving my bishop in front of my d pawn! (-40)" << std::endl;
                score -=40;
        }
	if(move.getPiece() == 'n' && (move.getStart().SquareName() != "b1" && move.getStart().SquareName() != "g1") && move.getTurn() < 6)
        {
		scores << "I'd be moving my knight again! (-25)" << std::endl;
                score -=25;
        }
	if(move.getPiece() == 'q' && move.getStart().SquareName() != "d1" && move.getTurn() < 6)
        {
		scores << "I'd be moving my queen again! (-25)" << std::endl;
                score -=25;
        }
	if(move.getPiece() == 'n' && (move.getEnd().getFile() == 'h' || move.getEnd().getFile() == 'a'))
	{
		scores << "I'd be moving my knight to the edge of the board (-25)" << std::endl;
		score-=25;
	}
	if(log.size() > 1 && move.getPiece() == 'p' && move.getEnd().SquareName() == "e4" && log.at(log.size()-2).getPiece() == 'p' && log.at(log.size()-2).getEnd().SquareName() == "d3")
	{
		scores << "This is a good follow up to d3! (+50)" << std::endl;
		score+=50;
	}
	if(move.getPiece() == 'p' && move.getTurn() > 30)
	{
		scores << "This pushes a pawn in the late game (+50)";
		score+=50;
	}
	if(move.getEnd().getRowIndex() < move.getStart().getRowIndex() && move.getStart().getRowIndex() <= 3)
	{
		scores << "This is a retreating move (-50)" << std::endl;
		score -= 50;
	}
	for(unsigned int i = log.size()-2; i > log.size() - 5; i--)
	{
		if(i >= 0 && i < log.size())
		{
			if((log.at(i).getEnd().SquareName() == move.getEnd().SquareName() || log.at(i).getStart().SquareName() == move.getEnd().SquareName()) && log.at(i).getPiece() == move.getPiece() && log.at(i).getColor() == move.getColor())
			{
				if(move.getStart().getAttack() == 0)
				{
					scores << "I would be going back to a square I was already on, but I am being attacked (-40)" << std::endl;
					score-=40;
				}
				else
				{
					scores << "I would be going back to a square I already was on! (-75)" << std::endl;
					score-=75;
				}
				break;
			}
		}
	}
	/*if(move.getEnd().SquareName() == "c3" && move.getPiece() == 'p' && move.getTurn() == 1)
	{
		scores << "Ryan told me too (+200)" << std::endl;
		score += 200;
	}*/

	board[move.getStart().getRowIndex()][move.getStart().getColumnIndex()].setPiece(move.getPiece(), move.getColor());
        board[move.getEnd().getRowIndex()][move.getEnd().getColumnIndex()] = scopy;
	if(move.getEnPassant())
        {
                board[4][enpassantcol].setPiece('p', 1);
        }
        if(move.getKingsCastle())
        {
                board[0][7].setPiece('r', move.getColor());
                board[0][5].deletePiece();
        }
        if(move.getQueensCastle())
        {
                board[0][0].setPiece('r', move.getColor());
                board[0][3].deletePiece();
        }
	score += rand()% 151;
	scores << "The final score for this move is " << score << "\n-------------------------------------------\n";
	return score;
}


void Chess::WhitetoMove()
{
	int decision = -1;
	std::vector<Square> moveable;
	std::vector<Move> validmoves;
	bool moved = false;
	int movecounter;
	int force = 0;
	int loops = 0;
	bool makemove = false;
	while(!moved)
	{
		movecounter = 0;
		if(loops == 1)
			makemove = true;
		loops++;
		for(int i = 0; i < 8; i++)
		{
			for(int j = 0; j < 8; j++)
			{
				if(board[i][j].getColor() == 0)
				{
					moveable.push_back(board[i][j]);
				}
			}
		}
		if(check)
		{
			log.at(log.size()-1).setCheck(true);
		}
		for(unsigned int i = 0; i < moveable.size(); i++)
		{
			if(moveable.at(i).getPiece() == 'p')
			{
				if(moveable.at(i).getRowIndex() + 1 < 8)
				{
					if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].IsEmpty())
					{
						if(!moved &&  ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex(), 'p', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								thoughts<< "Pawn to " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].SquareName()<<"!"<<std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex(), 'p', 0, ' ', true);
							}
							else if(makemove)
							{
								movecounter++;
							}
							else
							{
								if(moveable.at(i).getRowIndex()+1 == 7)
								{
									 Move move(turn, 0, 'p', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()], ' ', false, false, false, false, false, true);
									 move.setScore(CalcScore(move));
									  validmoves.push_back(move);
								}
								else
								{
									Move move(turn, 0, 'p', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()]);
									move.setScore(CalcScore(move));
									 validmoves.push_back(move);
								}
							}
						}
					}
				       	
				}
				if(moveable.at(i).getRowIndex() + 2 < 8 && moveable.at(i).getRowIndex() == 1 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].IsEmpty())
				{
					if(board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()].IsEmpty())
					{
				 		if(!moved && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex(), 'p', 0, ' ', false))
                                        	{
							if(makemove && decision == movecounter)
							{
                                                		thoughts<< "Pawn to " << board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()].SquareName()<<"!"<<std::endl;
                                                		moved = true;
								ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex(), 'p', 0, ' ', true);
							}
							else if(makemove)
							{
								movecounter++;
							}
							else
                                                        {
                                                                Move move(turn, 0, 'p', moveable.at(i), board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }

                                        	}
					}
	 
				}
				if(moveable.at(i).getRowIndex() + 1 < 8 && moveable.at(i).getColumnIndex() + 1 < 8)
				{
					if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].getColor() == 1)
					{
						if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+1, 'p', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								thoughts << "Pawn takes " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+1, 'p', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }

							else
                                                        {
                                                                Move move(turn, 0, 'p', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1], board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }

						}
					}
				}
				if(moveable.at(i).getRowIndex() + 1 < 8 && moveable.at(i).getColumnIndex() - 1 >= 0)
                                {
                                        if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-1, 'p', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Pawn takes " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-1, 'p', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'p', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1], board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }
				if(enpassant && (moveable.at(i).getColumnIndex() == enpassantcol - 1 || moveable.at(i).getColumnIndex() == enpassantcol + 1) && moveable.at(i).getRowIndex() == 4 && board[5][enpassantcol].IsEmpty())
				{
					if(!moved && ValidMove(5, enpassantcol, 'p', 0, ' ', false))
					{
						if(makemove && decision == movecounter)
						{
							thoughts << "Pawn takes " << board[5][enpassantcol].SquareName() << " becuase en passant is forced!" << std::endl;
							moved = true;
							ValidMove(5, enpassantcol, 'p', 0, ' ', true);
						}
						else if(makemove)
                                                {
                                                                movecounter++;
                                                }
						else
                                                {
                                                        Move move(turn, 0, 'p', moveable.at(i), board[5][enpassantcol], 'p', false, false, true);
                                                        move.setScore(CalcScore(move));
                                                        validmoves.push_back(move);
                                                }
					}
				}

			}
			if(moveable.at(i).getPiece() == 'n')
			{
				if(moveable.at(i).getRowIndex()+2 < 8 && moveable.at(i).getColumnIndex()+1 < 8)
				{
					if(board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1].IsEmpty())
					{
						if(!moved && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', false))
                                        	{
							if(makemove && decision == movecounter)
							{
                                                		thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1].SquareName()<<"!"<<std::endl;
                                                		moved = true;
								ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', true);
							}
							else if(makemove)
							{
								movecounter++;
							}
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                        	}
					}
					else if(board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1].getColor() == 1)
					{
						if(!moved && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {

                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1], board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
						}
					}

				}
				if(moveable.at(i).getRowIndex()+2 < 8 && moveable.at(i).getColumnIndex() - 1 >=0)
				{
					if(board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1].IsEmpty())
					{
						if(!moved && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', false))
                                        	{
							if(makemove && decision == movecounter)
							{
                                                		thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1].SquareName()<<"!"<<std::endl;
                                                		moved = true;
								ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                        	}
					}
					else if(board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
							{
								Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1], board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
							}
                                                }
                                        }


				}
				if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex()+2 < 8)
                                {
                                        if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2].SquareName()<<"!"<<std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }

					else if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2], board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }


                                }
                                if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex() - 2 >=0)
                                {
                                        if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2].SquareName()<<"!"<<std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2], board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }


                                }
				if(moveable.at(i).getRowIndex()-1 >= 0 && moveable.at(i).getColumnIndex()+2 < 8)
                                {
                                        if(board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', false) )
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2].SquareName()<<"!"<<std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else if(board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2], board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }


                                }
                                if(moveable.at(i).getRowIndex()-1 >= 0 && moveable.at(i).getColumnIndex() - 2 >=0)
                                {
                                        if(board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2].SquareName()<<"!"<<std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else if(board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2], board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }


                                }
				if(moveable.at(i).getRowIndex()-2 >= 0 && moveable.at(i).getColumnIndex()+1 < 8)
                                {
                                        if(board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1].SquareName()<<"!"<<std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else if(board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1], board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }


                                }
                                if(moveable.at(i).getRowIndex()-2 >= 0 && moveable.at(i).getColumnIndex() - 1 >=0)
                                {
                                        if(board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts<< "Knight to " << board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1].SquareName()<<"!"<<std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else if(board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1].getColor() == 1)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Knight takes " << board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
                                                        	moved = true;
								ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'n', moveable.at(i), board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1], board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }


                                }



			}
			if(moveable.at(i).getPiece() == 'q')
			{
				for(int x = 1; x <= (7 - moveable.at(i).getRowIndex()); x++)
				{
					if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].IsEmpty())
					{
						if(!moved && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								thoughts << "Queen to " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].SquareName() << "!"  <<std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', true);
							}
							else if(makemove)
							{
								movecounter++;
							}
							else
                                                        {
                                                                Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
						}
					}
					else
					{
						if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                        	{
                                                	if(!moved  && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
                                                	{
								if(makemove && decision == movecounter)
								{
                                                        		thoughts << "Queen takes " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
                                                        		moved = true;
									ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', true);
								}
								else if(makemove)
                                                        	{
                                                                	movecounter++;
                                                        	}
								else
                                                        	{
                                                                	Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()], board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].getPiece());
                                                                	move.setScore(CalcScore(move));
                                                                	validmoves.push_back(move);
                                                        	}
                                                	}
                                        	}

						break;
					}
				}
				for(int x = 1; x <= (7 - moveable.at(i).getColumnIndex()); x++)
				{
					if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].IsEmpty())
					{
						if(!moved  && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								thoughts << "Queen to " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
						}
					}
					else
					{
						if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                {
                                                        if(!moved && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Queen takes " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', true);
								}
								else if(makemove)
                                                        	{
                                                                	movecounter++;
                                                        	}
								else
                                                                {
                                                                        Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x], board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].getPiece());
                                                                        move.setScore(CalcScore(move));
                                                                        validmoves.push_back(move);
                                                                }
                                                        }
                                                }

						break;
					}
				}
				for(int x = moveable.at(i).getRowIndex() - 1; x >= 0; x--)
				{
					if(board[x][moveable.at(i).getColumnIndex()].IsEmpty())
					{
						if(!moved  && ValidMove(x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								thoughts << "Queen to " << board[x][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'q', moveable.at(i), board[x][moveable.at(i).getColumnIndex()]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
						}
					}
					else
					{
						if(board[x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                                {
                                                        if(!moved  && ValidMove(x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Queen takes " << board[x][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
                                                               	 	moved = true;
									ValidMove(x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', true);
								}
								else if(makemove)
                                                        	{
                                                                	movecounter++;
                                                        	}
								else
                                                                {
                                                                        Move move(turn, 0, 'q', moveable.at(i), board[x][moveable.at(i).getColumnIndex()], board[x][moveable.at(i).getColumnIndex()].getPiece());
                                                                        move.setScore(CalcScore(move));
                                                                        validmoves.push_back(move);
                                                                }
                                                        }
                                                }

						break;
					}
				}
		     		for(int x = moveable.at(i).getColumnIndex() - 1; x >= 0; x--)
	                 
				{	
					if(board[moveable.at(i).getRowIndex()][x].IsEmpty())
					{
						if(!moved  && ValidMove(moveable.at(i).getRowIndex(), x, 'q', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								thoughts << "Queen to " << board[moveable.at(i).getRowIndex()][x].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex(), x, 'q', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()][x]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
						}
					}
					else
					{
						if(board[moveable.at(i).getRowIndex()][x].getColor() == 1)
                                                {
                                                        if(!moved  && ValidMove(moveable.at(i).getRowIndex(), x, 'q', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Queen takes " << board[moveable.at(i).getRowIndex()][x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex(), x, 'q', 0, ' ', true);
								}
								else if(makemove)
                                                        	{
                                                                	movecounter++;
                                                        	}
								else
                                                                {
                                                                        Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()][x], board[moveable.at(i).getRowIndex()][x].getPiece());
                                                                        move.setScore(CalcScore(move));
                                                                        validmoves.push_back(move);
                                                                }
                                                        }
                                                }

						break;
					}
				}
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()-x >=0)
                                        {
                                                if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].IsEmpty())
                                                {
                                                        if(!moved  && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Queen to " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', true);
								}
								else if(makemove)
                                                        	{
                                                                	movecounter++;
                                                        	}
								else
                                                        	{
                                                                	Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x]);
                                                                	move.setScore(CalcScore(move));
                                                                	validmoves.push_back(move);
                                                        	}
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                	{
                                                        	if(!moved  && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        	{
									if(makemove && decision == movecounter)
									{
                                                                		thoughts << "Queen takes " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
                                                                		moved = true;
										ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                                		movecounter++;
                                                        		}
									else
                                                                	{
                                                                        	Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x], board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].getPiece());
                                                                        	move.setScore(CalcScore(move));
                                                                        	validmoves.push_back(move);
                                                                	}
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()+x <8 && moveable.at(i).getColumnIndex()+x <8)
                                        {
                                                if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                                {
                                                        if(!moved  && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Queen to " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', true);
								}
								else if(makemove)
        	                                                {
	                                                                movecounter++;
                	                                        }
								else
                                                        	{
                                                                	Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x]);
                                                               	 	move.setScore(CalcScore(move));
                                                                	validmoves.push_back(move);
                                                        	}
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                	{
                                                        	if(!moved  && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        	{
									if(makemove && decision == movecounter)
									{
                                                                		thoughts << "Queen takes " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                		moved = true;
										ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                                		movecounter++;
                                                        		}
									else
                                                                        {
                                                                                Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x], board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()+x < 8)
                                        {
                                                if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                                {
                                                        if(!moved  && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Queen to " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', true);
								}
								else if(makemove)
                                                        	{
                                                                	movecounter++;
                                                        	}
								else
                                                        	{
                                                                	Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x]);
                                                               	 	move.setScore(CalcScore(move));
                                                                	validmoves.push_back(move);
                                                        	}
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                	{
                                                        	if(!moved  && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        	{
									if(makemove && decision == movecounter)
									{
                                                                		thoughts << "Queen takes " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                		moved = true;
										ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                               			 movecounter++;
                                                        		}
									else
                                                                        {
                                                                                Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x], board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()+x <8 && moveable.at(i).getColumnIndex()-x >=0)
                                        {
                                                if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].IsEmpty())
                                                {
                                                        if(!moved  && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Queen to " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', true);
								}
								else if(makemove)
                                                        	{
                                                                	movecounter++;
                                                        	}
								else
                                                        	{
                                                                	Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x]);
                                                                	move.setScore(CalcScore(move));
                                                                	validmoves.push_back(move);
                                                        	}
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                	{
                                                        	if(!moved  && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        	{
									if(makemove && decision == movecounter)
									{
                                                                		thoughts << "Queen takes " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
                                                                		moved = true;
										ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                                		movecounter++;
                                                        		}
									else
                                                                        {
                                                                                Move move(turn, 0, 'q', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x], board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }




			}
			if(moveable.at(i).getPiece() == 'k')
			{
				if(cancastlekingside)
				{
					if(board[0][5].IsEmpty() && board[0][5].getAttack() == 0 && board[0][6].IsEmpty() && board[0][6].getAttack() == 0 && board[0][7].getPiece() == 'r' && board[0][7].getColor() == 0)
					{
						if(!moved && !check)
						{
							if(makemove && decision == movecounter)
							{
								moved = true;
								cancastlekingside = false;
								cancastlequeenside = false;
								thoughts << "I castle! (King's side)" << std::endl;
								board[0][6].setPiece('k', 0);
								board[0][5].setPiece('r', 0);
								board[0][4].deletePiece();
								board[0][7].deletePiece();
								Move move(turn, 'k', 0, board[0][4], board[0][6], ' ', true);
								log.push_back(move);
							}
							else if(makemove)
							{
								movecounter++;
							}
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[0][6], ' ', true);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
						}
					}

				}	

				if(cancastlequeenside)
				{
					if(board[0][3].IsEmpty() && board[0][3].getAttack() == 0 && board[0][2].IsEmpty() && board[0][2].getAttack() == 0 && board[0][1].IsEmpty() && board[0][0].getPiece() == 'r' && board[0][0].getColor() == 0)
                                        {
                                                if(!moved && !check)
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	moved = true;
                                                        	cancastlekingside = false;
                                                        	cancastlequeenside = false;
                                                        	thoughts << "I castle! (Queen's side)" << std::endl;
                                                        	board[0][2].setPiece('k', 0);
                                                        	board[0][3].setPiece('r', 0);
                                                        	board[0][4].deletePiece();
                                                        	board[0][0].deletePiece();
								Move move(turn, 'k', 0, board[0][4], board[0][2], ' ', false, true);
								log.push_back(move);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[0][2], ' ', false, true);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }

				}
				if(moveable.at(i).getRowIndex() + 1 < 8)
				{
					if(board[moveable.at(i).getRowIndex() + 1][moveable.at(i).getColumnIndex()].getColor() != 0 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].getAttack() == 0)
					{
						if(!moved  && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex(), 'k', 0, ' ', false))
						{
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].getColor() == 1)
								{
									thoughts << "King takes " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
								}
								else
								{
									thoughts << "King to " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex(), 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()], board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
						}
					}
				}
				if(moveable.at(i).getRowIndex() - 1 >= 0)
                                {
                                        if(board[moveable.at(i).getRowIndex() - 1][moveable.at(i).getColumnIndex()].getColor() != 0 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()].getAttack() == 0)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex(), 'k', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()].getColor() == 1)
								{
                                                                	thoughts << "King takes " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
								}
                                                        	else
								{
                                                        		thoughts << "King to " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex(), 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()], board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }
				if(moveable.at(i).getColumnIndex() + 1 < 8)
                                {
                                        if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1].getColor() != 0 && board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1].getAttack() == 0)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1].getColor() == 1)
								{
                                                                	thoughts << "King takes " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								}
                                                        	else
								{
                                                   			thoughts << "King to " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1], board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }
				if(moveable.at(i).getColumnIndex() - 1 >= 0)
                                {
                                        if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1].getColor() != 0 && board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1].getAttack() == 0)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1].getColor() == 1)
								{
                                                                	thoughts << "King takes " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
								}
                                                        	else
								{
                                                        		thoughts << "King to " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1], board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }
				if(moveable.at(i).getColumnIndex() - 1 >= 0 && moveable.at(i).getRowIndex() - 1 >= 0)
                                {
                                        if(board[moveable.at(i).getRowIndex() - 1][moveable.at(i).getColumnIndex() - 1].getColor() != 0 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-1].getAttack() == 0)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-1].getColor() == 1)
								{
                                                                	thoughts << "King takes " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
								}
                                                        	else
								{
                                                        		thoughts << "King to " << board[moveable.at(i).getRowIndex() - 1][moveable.at(i).getColumnIndex() - 1].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-1], board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }
				if(moveable.at(i).getRowIndex() + 1 < 8 && moveable.at(i).getColumnIndex() + 1 < 8)
                                {
                                        if(board[moveable.at(i).getRowIndex() + 1][moveable.at(i).getColumnIndex()+1].getColor() != 0&& board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].getAttack() == 0)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].getColor() == 1)
								{
                                                                	thoughts << "King takes " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								}
                                                        	else
								{
                                                        		thoughts << "King to " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1], board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }
				if(moveable.at(i).getRowIndex() + 1 < 8 && moveable.at(i).getColumnIndex() - 1 >= 0)
                                {
                                        if(board[moveable.at(i).getRowIndex() + 1][moveable.at(i).getColumnIndex() - 1].getColor() != 0 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].getAttack() == 0)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].getColor() == 1)
								{
                                                                	thoughts << "King takes " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
								}
                                                        	else 
								{
                                                       			thoughts << "King to " << board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1], board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }

				if(moveable.at(i).getRowIndex() - 1 >= 0 && moveable.at(i).getColumnIndex() + 1 < 8)
                                {
                                        if(board[moveable.at(i).getRowIndex() - 1][moveable.at(i).getColumnIndex()+1].getColor() != 0 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+1].getAttack() == 0)
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
								if(board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+1].getColor() == 1)
								{
                                                                	thoughts << "King takes " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								}
                                                        	else
								{
                                                        		thoughts << "King to " << board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+1].SquareName() << "!" << std::endl;
								}
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'k', moveable.at(i), board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+1], board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+1].getPiece());
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
                                }


			}
			if(moveable.at(i).getPiece() == 'b')
			{
				for(int x = 1; x < 8; x++)
				{
					if(moveable.at(i).getRowIndex()+x < 8 && moveable.at(i).getColumnIndex()+x < 8)
					{
						if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].IsEmpty())
						{
							if(!moved && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
							{
								if(makemove && decision == movecounter)
								{
									thoughts << "Bishop to "<< board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
									moved = true;
									ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', true);
								}
								else if(makemove)
                                                                {
                                                                        movecounter++;
                                                                }
								else
                                                        	{
                                                                	Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x]);
                                                                	move.setScore(CalcScore(move));
                                                                	validmoves.push_back(move);
                                                        	}
							}
						}
						else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Bishop takes " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', true);
									}
									else if(makemove)
                                                                        {
                                                                                movecounter++;
                                                                        }
									else
                                                                	{
                                                                        	Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x], board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].getPiece());
                                                                        	move.setScore(CalcScore(move));
                                                                        	validmoves.push_back(move);
                                                                	}
                                                                }
                                                        }


							break;
						}
					}
				}
				for(int x = 1; x < 8; x++)
				{
					if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()-x >=0)
					{
						if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].IsEmpty())
						{
							if(!moved && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
							{
								if(makemove && decision == movecounter)
								{
									thoughts << "Bishop to " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
									moved = true;
									ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', true);
								}
								else if(makemove)
                                                                {
                                                                        movecounter++;
                                                                }
								else
                                                                {
                                                                        Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x]);
                                                                        move.setScore(CalcScore(move));
                                                                        validmoves.push_back(move);
                                                                }
							}
						}
						else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Bishop takes " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', true);
									}
									else if(makemove)
                                                                        {
                                                                                movecounter++;
                                                                        }
									else
                                                                        {
                                                                                Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x], board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }

                                                                }
                                                        }

							break;
						}
					}
				}
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()+x <8)
                                        {
                                                if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                                {
                                                        if(!moved && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Bishop to " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', true);
								}
								else if(makemove)
                                                                {
                                                                        movecounter++;
                                                                }
								else
                                                                {
                                                                        Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x]);
                                                                        move.setScore(CalcScore(move));
                                                                        validmoves.push_back(move);
                                                                }
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Bishop takes " << board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', true);
									}
									else if(makemove)
                                                                        {
                                                                                movecounter++;
                                                                        }
									else
                                                                        {
                                                                                Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x], board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                                }
                                                        }

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()+x <8 && moveable.at(i).getColumnIndex()-x >=0)
                                        {
                                                if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].IsEmpty())
                                                {
                                                        if(!moved && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
                                                        {
								if(makemove && decision == movecounter)
								{
                                                                	thoughts << "Bishop to " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
                                                                	moved = true;
									ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', true);
								}
								else if(makemove)
                                                                {
                                                                        movecounter++;
                                                                }
								else
                                                                {
                                                                        Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x]);
                                                                        move.setScore(CalcScore(move));
                                                                        validmoves.push_back(move);
                                                                }
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Bishop takes " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', true);
									}
									else if(makemove)
									{
										movecounter++;
									}
									else
                                                                        {
                                                                                Move move(turn, 0, 'b', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x], board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                                }
                                                        }

                                                        break;
						}
                                        }
                                }

	
			}
			if(moveable.at(i).getPiece() == 'r')
			{
				 for(int x = 1; x <= (7 - moveable.at(i).getRowIndex()); x++)
                                {
                                        if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Rook to " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].SquareName() << "!"  <<std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }

							else
                                                        {
                                                        	Move move(turn, 0, 'r', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()]);
                                                        	move.setScore(CalcScore(move));
                                                        	validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else
					{
						if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Rook takes " << board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                                		movecounter++;
                                                        		}
									else
                                                        		{
                                                                		Move move(turn, 0, 'r', moveable.at(i), board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()], board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].getPiece());
                                                                		move.setScore(CalcScore(move));
                                                                		validmoves.push_back(move);
                                                        		}
                                                                }
                                                        }

						break;
					}
                                }
                                for(int x = 1; x <= (7 - moveable.at(i).getColumnIndex()); x++)
                                {
                                        if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'r', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Rook to " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'r', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'r', moveable.at(i), board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else
					{
						if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'r', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Rook takes " << board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'r', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                               			 movecounter++;
                                                        		}
									else
                                                                        {
                                                                                Move move(turn, 0, 'r', moveable.at(i), board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x], board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                                }
                                                        }
	
						break;
					}
                                }
                                for(int x = moveable.at(i).getRowIndex() - 1; x >= 0; x--)
                                {
                                        if(board[x][moveable.at(i).getColumnIndex()].IsEmpty())
                                        {
                                                if(!moved && ValidMove(x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Rook to " << board[x][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'r', moveable.at(i), board[x][moveable.at(i).getColumnIndex()]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else
					{
						if(board[x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Rook takes " << board[x][moveable.at(i).getColumnIndex()].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                                		movecounter++;
                                                        		}
									else
                                                                        {
                                                                                Move move(turn, 0, 'r', moveable.at(i), board[x][moveable.at(i).getColumnIndex()], board[x][moveable.at(i).getColumnIndex()].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                                }
                                                        }

						break;
					}
                                }
                                for(int x = moveable.at(i).getColumnIndex() - 1; x >= 0; x--)

                                {
                                        if(board[moveable.at(i).getRowIndex()][x].IsEmpty())
                                        {
                                                if(!moved && ValidMove(moveable.at(i).getRowIndex(), x, 'r', 0, ' ', false))
                                                {
							if(makemove && decision == movecounter)
							{
                                                        	thoughts << "Rook to " << board[moveable.at(i).getRowIndex()][x].SquareName() << "!" << std::endl;
								moved = true;
								ValidMove(moveable.at(i).getRowIndex(), x, 'r', 0, ' ', true);
							}
							else if(makemove)
                                                        {
                                                                movecounter++;
                                                        }
							else
                                                        {
                                                                Move move(turn, 0, 'r', moveable.at(i), board[moveable.at(i).getRowIndex()][x]);
                                                                move.setScore(CalcScore(move));
                                                                validmoves.push_back(move);
                                                        }
                                                }
                                        }
					else 
					{
						if(board[moveable.at(i).getRowIndex()][x].getColor() == 1)
                                                        {
                                                                if(!moved && ValidMove(moveable.at(i).getRowIndex(), x, 'r', 0, ' ', false))
                                                                {
									if(makemove && decision == movecounter)
									{
                                                                        	thoughts << "Rook takes " << board[moveable.at(i).getRowIndex()][x].SquareName() << "!" << std::endl;
                                                                        	moved = true;
										ValidMove(moveable.at(i).getRowIndex(), x, 'r', 0, ' ', true);
									}
									else if(makemove)
                                                        		{
                                                                		movecounter++;
                                                        		}
									else
                                                                        {
                                                                                Move move(turn, 0, 'r', moveable.at(i), board[moveable.at(i).getRowIndex()][x], board[moveable.at(i).getRowIndex()][x].getPiece());
                                                                                move.setScore(CalcScore(move));
                                                                                validmoves.push_back(move);
                                                                        }
                                                                }
                                                        }

						break;
					}
                                }

			}
		}
		if(moved)
		{
			thoughts << std::endl;
			break;
		}
		decision = Decide(force, validmoves);
		DefenseReset();
		if(decision == -1 && check)
		{
			checkmate = true;
			moved = true;
			break;
		}
		else if(decision == -1)
		{
			stalemate = true;
			moved = true;
			break;
		}
	}
}

bool Chess::DidIWin(bool checkfurther, bool mateintwo, bool blackmateintwo)
{
	bool blackmateinone = false;
	bool allmovesaremate = false;
	bool foundamateline = false;
	if(mateintwo)
		allmovesaremate = true;
	std::vector<Square> moveable;
   	for(int i = 0; i < 8; i++)
   	{
      		for(int j = 0; j < 8; j++)
      		{
         		if(board[i][j].getColor() == 1)
         		{
            			moveable.push_back(board[i][j]);
            		}
         	}
    	}
	int nummoves = 0;
	for(Square s : moveable)
	{
		if(s.getPiece() == 'p')
		{
			if(s.getRowIndex()-1 >= 0 && board[s.getRowIndex()-1][s.getColumnIndex()].IsEmpty())
			{
				if(s.getRowIndex()-1 == 0)
				{
					board[s.getRowIndex()-1][s.getColumnIndex()].setPiece('q', 1);
				}
				else
					board[s.getRowIndex()-1][s.getColumnIndex()].setPiece('p', 1);
				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
				DefenseReset();
				if(!blackcheck)
				{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
						{
							foundamateline = true;
						}
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
						{
							allmovesaremate = false;
						}
						if(!blackmateinone && checkfurther && !HasAValidMove())
							blackmateinone = true;	
						nummoves++;
				}
				board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 1);
				board[s.getRowIndex()-1][s.getColumnIndex()].deletePiece();

			}
			if(s.getRowIndex() == 6 && board[s.getRowIndex()-2][s.getColumnIndex()].IsEmpty() && board[s.getRowIndex()-1][s.getColumnIndex()].IsEmpty())
			{
				board[s.getRowIndex()-2][s.getColumnIndex()].setPiece('p', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
			   	if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                              	}
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;	
               				nummoves++;
				}
           	 		board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 1);
            			board[s.getRowIndex()-2][s.getColumnIndex()].deletePiece();
			}
			if(s.getColumnIndex() + 1 < 8)
			{
				if(s.getRowIndex()-1 >= 0 && board[s.getRowIndex()-1][s.getColumnIndex()+1].getColor() == 0)
				{
					Square scopy = board[s.getRowIndex()-1][s.getColumnIndex()+1];
					if(s.getRowIndex() - 1 == 0)
					{
						board[s.getRowIndex()-1][s.getColumnIndex()+1].setPiece('q', 1);
					}
					else
						board[s.getRowIndex()-1][s.getColumnIndex()+1].setPiece('p', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
               				DefenseReset();
				   	if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 1);
               				board[s.getRowIndex()-1][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 0);
				}
			}
			if(s.getColumnIndex() - 1 >= 0)
			{
				if(s.getRowIndex()-1 >= 0 && board[s.getRowIndex()-1][s.getColumnIndex()-1].getColor() == 0)
				{
					Square scopy = board[s.getRowIndex()-1][s.getColumnIndex()-1];
					if(board[s.getRowIndex() - 1] == 0)
					{
						board[s.getRowIndex()-1][s.getColumnIndex()-1].setPiece('q', 1);
					}
					else
						board[s.getRowIndex()-1][s.getColumnIndex()-1].setPiece('p', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
               				DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 1);
               				board[s.getRowIndex()-1][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 0);
				}
			}
			if(s.getRowIndex() == 3  && enpassant && (s.getColumnIndex() - 1 == enpassantcol || s.getColumnIndex() + 1 == enpassantcol) && board[2][enpassantcol].IsEmpty())
         		{
            			Square scopy = board[3][enpassantcol];
            			board[2][enpassantcol].setPiece('p', 1);
            			board[3][enpassantcol].deletePiece();
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
					nummoves++;
				}
            			board[2][enpassantcol].deletePiece();
            			board[3][enpassantcol] = scopy;
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 1);
			}

		}
		else if(s.getPiece() == 'k')
		{
			if(s.getRowIndex() + 1 < 8 && board[s.getRowIndex()+1][s.getColumnIndex()].getColor() != 1)
         		{
            			bool capture = false;
				Square scopy;
				if(board[s.getRowIndex()+1][s.getColumnIndex()].getColor() == 0)
				{
               				capture = true;
					scopy = board[s.getRowIndex()+1][s.getColumnIndex()];
				}
               			board[s.getRowIndex()+1][s.getColumnIndex()].setPiece('k', 1);
               			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
               			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                      			nummoves++;
				}
				if(capture)
				{
					board[s.getRowIndex()+1][s.getColumnIndex()].setPiece(scopy.getPiece(), 0);
				}
				else
				{
					board[s.getRowIndex()+1][s.getColumnIndex()].deletePiece();
				}
               			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);


          		}
			if(s.getRowIndex() + 1 < 8 && s.getColumnIndex() + 1 < 8 && board[s.getRowIndex()+1][s.getColumnIndex()+1].getColor() != 1)
         		{
				Square scopy;
            			bool capture = false;
            			if(board[s.getRowIndex()+1][s.getColumnIndex()+1].getColor() == 0)
            			{
               				capture = true;
               				scopy = board[s.getRowIndex()+1][s.getColumnIndex()+1];
            			}
            			board[s.getRowIndex()+1][s.getColumnIndex()+1].setPiece('k', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
			   	if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
               				nummoves++;
				}
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);
			   	if(capture)
					board[s.getRowIndex()+1][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 0);
				else
               				board[s.getRowIndex()+1][s.getColumnIndex()+1].deletePiece();
          		}
			if(s.getRowIndex() + 1 < 8 && s.getColumnIndex() - 1 >= 0 && board[s.getRowIndex()+1][s.getColumnIndex()-1].getColor() != 1)
         		{
			   	Square scopy;
            			bool capture = false;
            			if(board[s.getRowIndex()+1][s.getColumnIndex()-1].getColor() == 0)
            			{
               				capture = true;
               				scopy = board[s.getRowIndex()+1][s.getColumnIndex()-1];
            			}
            			board[s.getRowIndex()+1][s.getColumnIndex()-1].setPiece('k', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
               				nummoves++;
				}
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);
				if(capture)
					board[s.getRowIndex()+1][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 0);
				else
               				board[s.getRowIndex()+1][s.getColumnIndex()-1].deletePiece();
         		}
			if(s.getRowIndex() - 1 >=0  && s.getColumnIndex() - 1 >= 0 && board[s.getRowIndex()-1][s.getColumnIndex()-1].getColor() != 1)
         		{
            			Square scopy;
            			bool capture = false;
            			if(board[s.getRowIndex()-1][s.getColumnIndex()-1].getColor() == 0)
            			{
               				capture = true;
               				scopy = board[s.getRowIndex()-1][s.getColumnIndex()-1];
            			}
            			board[s.getRowIndex()-1][s.getColumnIndex()-1].setPiece('k', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
               				nummoves++;
				}
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);
				if(capture)
					board[s.getRowIndex()-1][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 0);
				else
               				board[s.getRowIndex()-1][s.getColumnIndex()-1].deletePiece();
          		}
			if(s.getRowIndex() - 1 >=0 && s.getColumnIndex() + 1 < 8 && board[s.getRowIndex()-1][s.getColumnIndex()+1].getColor() != 1)
         		{
				Square scopy;
            			bool capture = false;
            			if(board[s.getRowIndex()-1][s.getColumnIndex()+1].getColor() == 0)
            			{
               				capture = true;
               				scopy = board[s.getRowIndex()-1][s.getColumnIndex()+1];
            			}
            			board[s.getRowIndex()-1][s.getColumnIndex()+1].setPiece('k', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
               				nummoves++;
				}
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);
				if(capture)
						board[s.getRowIndex()-1][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 0);
				else
               					board[s.getRowIndex()-1][s.getColumnIndex()+1].deletePiece();
          		}
			if(s.getColumnIndex() + 1 < 8 && board[s.getRowIndex()][s.getColumnIndex()+1].getColor() != 1)
         		{
				Square scopy;
            			bool capture = false;
            			if(board[s.getRowIndex()][s.getColumnIndex()+1].getColor() == 0)
            			{
               				capture = true;
               				scopy = board[s.getRowIndex()][s.getColumnIndex()+1];
            			}
            			board[s.getRowIndex()][s.getColumnIndex()+1].setPiece('k', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
               				nummoves++;
				}
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);
				if(capture)
					board[s.getRowIndex()][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 0);
				else
               				board[s.getRowIndex()][s.getColumnIndex()+1].deletePiece();
         		}
			if(s.getRowIndex() - 1 >= 0 && board[s.getRowIndex()-1][s.getColumnIndex()].getColor() != 1)
         		{
				Square scopy;
            			bool capture = false;
            			if(board[s.getRowIndex()-1][s.getColumnIndex()].getColor() == 0)
            			{
               				capture = true;
               				scopy = board[s.getRowIndex()-1][s.getColumnIndex()];
            			}
            			board[s.getRowIndex()-1][s.getColumnIndex()].setPiece('k', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
               				nummoves++;
				}
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);
				if(capture)
					board[s.getRowIndex()-1][s.getColumnIndex()].setPiece(scopy.getPiece(), 0);
				else
               				board[s.getRowIndex()-1][s.getColumnIndex()].deletePiece();
         		}
			if(s.getColumnIndex() - 1 >= 0 && board[s.getRowIndex()][s.getColumnIndex()-1].getColor() != 1)
         		{
				Square scopy;
            			bool capture = false;
            			if(board[s.getRowIndex()][s.getColumnIndex()-1].getColor() == 0)
            			{
               				capture = true;
               				scopy = board[s.getRowIndex()][s.getColumnIndex()-1];
            			}
            			board[s.getRowIndex()][s.getColumnIndex()-1].setPiece('k', 1);
            			board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
            			DefenseReset();
				if(!blackcheck)
				{
					if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
					if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
					if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
               				nummoves++;
				}
            			board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 1);
				if(capture)
					board[s.getRowIndex()][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 0);
				else
               				board[s.getRowIndex()][s.getColumnIndex()-1].deletePiece();
          		}

		}
		else if(s.getPiece() == 'n')
		{
			if(s.getRowIndex()+2 < 8 && s.getColumnIndex()+1 < 8)
         		{
            			if(board[s.getRowIndex()+2][s.getColumnIndex()+1].getColor() != 1)
            			{
					Square scopy;
               				bool capture = false;
               				if(board[s.getRowIndex()+2][s.getColumnIndex()+1].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()+2][s.getColumnIndex()+1];
               				}
               				board[s.getRowIndex()+2][s.getColumnIndex()+1].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()+2][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()+2][s.getColumnIndex()+1].deletePiece();
               			}
          		}
			if(s.getRowIndex()+2 < 8 && s.getColumnIndex()-1 >= 0)
         		{
            			if(board[s.getRowIndex()+2][s.getColumnIndex()-1].getColor() != 1)
            			{
					Square scopy;
               				bool capture = false;
               				if(board[s.getRowIndex()+2][s.getColumnIndex()-1].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()+2][s.getColumnIndex()-1];
               				}
               				board[s.getRowIndex()+2][s.getColumnIndex()-1].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()+2][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()+2][s.getColumnIndex()-1].deletePiece();
               			}


         		}
			if(s.getRowIndex()+1 < 8 && s.getColumnIndex()+2 < 8)
         		{
            			if(board[s.getRowIndex()+1][s.getColumnIndex()+2].getColor() != 1)
            			{
					Square scopy;
               				bool capture = false;
               				if(board[s.getRowIndex()+1][s.getColumnIndex()+2].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()+1][s.getColumnIndex()+2];
               				}
               				board[s.getRowIndex()+1][s.getColumnIndex()+2].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()+1][s.getColumnIndex()+2].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()+1][s.getColumnIndex()+2].deletePiece();
               			}

         		}
			if(s.getRowIndex()+1 < 8 && s.getColumnIndex()-2 >= 0)
         		{
            			if(board[s.getRowIndex()+1][s.getColumnIndex()-2].getColor() != 1)
            			{
					Square scopy;
              	 			bool capture = false;
               				if(board[s.getRowIndex()+1][s.getColumnIndex()-2].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()+1][s.getColumnIndex()-2];
               				}
               				board[s.getRowIndex()+1][s.getColumnIndex()-2].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()+1][s.getColumnIndex()-2].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()+1][s.getColumnIndex()-2].deletePiece();
               			}

         		}
			if(s.getRowIndex()-1 >= 0 && s.getColumnIndex()+2 < 8)
         		{
				if(board[s.getRowIndex()-1][s.getColumnIndex()+2].getColor() != 1)
            			{
					Square scopy;
               				bool capture = false;
               				if(board[s.getRowIndex()-1][s.getColumnIndex()+2].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()-1][s.getColumnIndex()+2];
               				}
               				board[s.getRowIndex()-1][s.getColumnIndex()+2].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()-1][s.getColumnIndex()+2].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()-1][s.getColumnIndex()+2].deletePiece();
               			}

         		}
			if(s.getRowIndex()-1 >= 0 && s.getColumnIndex()-2 >= 0)
         		{
            			if(board[s.getRowIndex()-1][s.getColumnIndex()-2].getColor() != 1)
            			{
					Square scopy;
               				bool capture = false;
               				if(board[s.getRowIndex()-1][s.getColumnIndex()-2].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()-1][s.getColumnIndex()-2];
               				}
               				board[s.getRowIndex()-1][s.getColumnIndex()-2].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()-1][s.getColumnIndex()-2].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()-1][s.getColumnIndex()-2].deletePiece();
               			}
         		}
			if(s.getRowIndex()-2 >= 0 && s.getColumnIndex()+1 < 8)
         		{
            			if(board[s.getRowIndex()-2][s.getColumnIndex()+1].getColor() != 1)
            			{
					Square scopy;
               				bool capture = false;
               				if(board[s.getRowIndex()-2][s.getColumnIndex()+1].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()-2][s.getColumnIndex()+1];
               				}
               				board[s.getRowIndex()-2][s.getColumnIndex()+1].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()-2][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()-2][s.getColumnIndex()+1].deletePiece();
               			}


         		}
			if(s.getRowIndex()-2 >= 0 && s.getColumnIndex()-1 >= 0)
         		{
            			if(board[s.getRowIndex()-2][s.getColumnIndex()-1].getColor() != 1)
            			{
					Square scopy;
               				bool capture = false;
               				if(board[s.getRowIndex()-2][s.getColumnIndex()-1].getColor() == 0)
               				{
                  				capture = true;
                  				scopy = board[s.getRowIndex()-2][s.getColumnIndex()-1];
               				}
               				board[s.getRowIndex()-2][s.getColumnIndex()-1].setPiece('n', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                     				nummoves++;
					}
					board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',1);
					if(capture)
						board[s.getRowIndex()-2][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 0);
					else
						board[s.getRowIndex()-2][s.getColumnIndex()-1].deletePiece();
               			}

          		}

		}
		else if(s.getPiece() == 'q')
		{
			for(int x = 1; x <= (7 - s.getRowIndex()); x++)
         		{
            			if(board[s.getRowIndex()+x][s.getColumnIndex()].IsEmpty())
            			{
               				board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('q', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[s.getRowIndex()+x][s.getColumnIndex()].deletePiece();
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
             			}
             			else
             			{
               				if(board[s.getRowIndex()+x][s.getColumnIndex()].getColor() == 0)
                			{
				      		Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()];
                  				board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   	DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                       	 	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
								blackmateinone = true;
                     					nummoves++;
						}
						board[s.getRowIndex()+x][s.getColumnIndex()].setPiece(scopy.getPiece(), 0);
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
				   	}
              				break;
             			}
         		}
			for(int x = 1; x <= (7 - s.getColumnIndex()); x++)
         		{
            			if(board[s.getRowIndex()][s.getColumnIndex()+x].IsEmpty())
            			{
               				board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('q', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[s.getRowIndex()][s.getColumnIndex()+x].deletePiece();
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
             			}
             			else
             			{
               				if(board[s.getRowIndex()][s.getColumnIndex()+x].getColor() == 0)
                			{
				      		Square scopy = board[s.getRowIndex()][s.getColumnIndex()+x];
                  				board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   	DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
						board[s.getRowIndex()][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 0);
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
				   	}
              				break;
             			}
         		}
			for(int x = s.getRowIndex() - 1; x >= 0; x--)
         		{
            			if(board[x][s.getColumnIndex()].IsEmpty())
            			{
               				board[x][s.getColumnIndex()].setPiece('q', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[x][s.getColumnIndex()].deletePiece();
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
             			}
             			else
             			{
               				if(board[x][s.getColumnIndex()].getColor() == 0)
                			{
				      		Square scopy = board[x][s.getColumnIndex()];
                  				board[x][s.getColumnIndex()].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   	DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
						board[x][s.getColumnIndex()].setPiece(scopy.getPiece(), 0);
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
				   	}
              				break;
             			}
         		}
			for(int x = s.getColumnIndex() - 1; x >= 0; x--)
         		{
            			if(board[s.getRowIndex()][x].IsEmpty())
            			{
               				board[s.getRowIndex()][x].setPiece('q', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[s.getRowIndex()][x].deletePiece();
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
             			}
             			else
             			{
               				if(board[s.getRowIndex()][x].getColor() == 0)
                			{
				      		Square scopy = board[s.getRowIndex()][x];
                  				board[s.getRowIndex()][x].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   	DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
						board[s.getRowIndex()][x].setPiece(scopy.getPiece(), 0);
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
				   	}
              				break;
             			}
         		}
			for(int x = 1; x < 8; x++)
         		{
				if(s.getRowIndex() - x >= 0 && s.getColumnIndex() - x >= 0)
				{
               				if(board[s.getRowIndex()-x][s.getColumnIndex()-x].IsEmpty())
               				{
                  				board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()-x][s.getColumnIndex()-x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
               				}
               				else
               				{
                  				if(board[s.getRowIndex()-x][s.getColumnIndex()-x].getColor() == 0)
                  				{
						   	Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()-x];
                     					board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('q', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
                   				}
                   				break;
						PrintBoard();
				   	}
         			}
			}
			for(int x = 1; x < 8; x++)
         		{
         			if(s.getRowIndex() - x >= 0 && s.getColumnIndex() + x < 8)
				{
               				if(board[s.getRowIndex()-x][s.getColumnIndex()+x].IsEmpty())
               				{
                  				board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                               		{	
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()-x][s.getColumnIndex()+x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
               				}
               				else
               				{	
                  				if(board[s.getRowIndex()-x][s.getColumnIndex()+x].getColor() == 0)
                  				{
						   	Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()+x];
                     					board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('q', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
                   				}
                   				break;
				   	}
         			}
			}
			for(int x = 1; x < 8; x++)
         		{
            			if(s.getRowIndex() + x < 8 && s.getColumnIndex() - x >= 0)
				{
               				if(board[s.getRowIndex()+x][s.getColumnIndex()-x].IsEmpty())
               				{
                  				board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()+x][s.getColumnIndex()-x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
               				}
               				else
               				{
                  				if(board[s.getRowIndex()+x][s.getColumnIndex()-x].getColor() == 0)
                  				{
							Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()-x];
                     					board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('q', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
                   				}
                   				break;
				   	}
         			}
			}
			for(int x = 1; x < 8; x++)
         		{
            			if(s.getRowIndex() + x < 8 && s.getColumnIndex() + x < 8)
				{
               				if(board[s.getRowIndex()+x][s.getColumnIndex()+x].IsEmpty())
               				{
                  				board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('q', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()+x][s.getColumnIndex()+x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
               				}
               				else
               				{
                  				if(board[s.getRowIndex()+x][s.getColumnIndex()+x].getColor() == 0)
                  				{
						   	Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()+x];
                     					board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('q', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 1);
                   				}
                   				break;
				   	}

         			}
			}

		}
		else if(s.getPiece() == 'b')
		{
         		for(int x = 1; x < 8; x++)
         		{
				if(s.getRowIndex() - x >= 0 && s.getColumnIndex() - x >= 0)
				{
               				if(board[s.getRowIndex()-x][s.getColumnIndex()-x].IsEmpty())
               				{
                  				board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('b', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{	
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()-x][s.getColumnIndex()-x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
               				}
               				else
               				{
                  				if(board[s.getRowIndex()-x][s.getColumnIndex()-x].getColor() == 0)
                  				{
						   	Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()-x];
                     					board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('b', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
                   				}
                   				break;
				   	}
         			}
			}
			for(int x = 1; x < 8; x++)
         		{
         			if(s.getRowIndex() - x >= 0 && s.getColumnIndex() + x < 8)
				{
               				if(board[s.getRowIndex()-x][s.getColumnIndex()+x].IsEmpty())
               				{
                  				board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('b', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()-x][s.getColumnIndex()+x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
               				}
               				else
               				{
                  				if(board[s.getRowIndex()-x][s.getColumnIndex()+x].getColor() == 0)
                  				{
						   	Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()+x];
                     					board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('b', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
                   				}
                   				break;
				   	}
         			}
			}
			for(int x = 1; x < 8; x++)
         		{
            			if(s.getRowIndex() + x < 8 && s.getColumnIndex() - x >= 0)
				{
               				if(board[s.getRowIndex()+x][s.getColumnIndex()-x].IsEmpty())
               				{
                  				board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('b', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()+x][s.getColumnIndex()-x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
               				}
               				else
               				{
                  				if(board[s.getRowIndex()+x][s.getColumnIndex()-x].getColor() == 0)
                  				{
							Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()-x];
                     					board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('b', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
                   				}
                   				break;
				 	}
         			}
			}
			for(int x = 1; x < 8; x++)
         		{
            			if(s.getRowIndex() + x < 8 && s.getColumnIndex() + x < 8)
				{
               				if(board[s.getRowIndex()+x][s.getColumnIndex()+x].IsEmpty())
               				{
                  				board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('b', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                  				DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
                  				board[s.getRowIndex()+x][s.getColumnIndex()+x].deletePiece();
                  				board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
               				}
               				else
               				{
                  				if(board[s.getRowIndex()+x][s.getColumnIndex()+x].getColor() == 0)
                  				{
						   	Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()+x];
                     					board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('b', 1);
                     					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                     					DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                        					nummoves++;
							}
                     					board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 0);
                     					board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 1);
                   				}
                   				break;
				   	}

         			}
			}

		}
		else if(s.getPiece() == 'r')
		{
			for(int x = 1; x <= (7 - s.getRowIndex()); x++)
         		{
            			if(board[s.getRowIndex()+x][s.getColumnIndex()].IsEmpty())
            			{
               				board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('r', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                 				 nummoves++;
					}
               				board[s.getRowIndex()+x][s.getColumnIndex()].deletePiece();
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
             			}
             			else
             			{
               				if(board[s.getRowIndex()+x][s.getColumnIndex()].getColor() == 0)
                			{
				      		Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()];
                  				board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('r', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   	DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
						board[s.getRowIndex()+x][s.getColumnIndex()].setPiece(scopy.getPiece(), 0);
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
				   	}
              				break;
             			}
         		}
			for(int x = 1; x <= (7 - s.getColumnIndex()); x++)
         		{
            			if(board[s.getRowIndex()][s.getColumnIndex()+x].IsEmpty())
            			{
               				board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('r', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[s.getRowIndex()][s.getColumnIndex()+x].deletePiece();
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
             			}
             			else
             			{
               				if(board[s.getRowIndex()][s.getColumnIndex()+x].getColor() == 0)
                			{
				      		Square scopy = board[s.getRowIndex()][s.getColumnIndex()+x];
                  				board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('r', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   	DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
						board[s.getRowIndex()][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 0);
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
				   	}
              				break;
             			}
         		}
			for(int x = s.getRowIndex() - 1; x >= 0; x--)
         		{
            			if(board[x][s.getColumnIndex()].IsEmpty())
            			{
               				board[x][s.getColumnIndex()].setPiece('r', 1);
               				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!blackcheck)
					{
						if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                {
                                                        foundamateline = true;
                                                }
						if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                {
                                                        allmovesaremate = false;
                                                }
						if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        blackmateinone = true;
                  				nummoves++;
					}
               				board[x][s.getColumnIndex()].deletePiece();
               				board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
             			}
             			else
             			{
               				if(board[x][s.getColumnIndex()].getColor() == 0)
                			{
				      		Square scopy = board[x][s.getColumnIndex()];
                  				board[x][s.getColumnIndex()].setPiece('r', 1);
                  				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   	DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                     					nummoves++;
						}
						board[x][s.getColumnIndex()].setPiece(scopy.getPiece(), 0);
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
				   	}
              				break;
             			}		
         		}
			for(int x = s.getColumnIndex() - 1; x >= 0; x--)
         		{
            				if(board[s.getRowIndex()][x].IsEmpty())
            				{
               					board[s.getRowIndex()][x].setPiece('r', 1);
               					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
						DefenseReset();
						if(!blackcheck)
						{
							if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                	{
                                                        	foundamateline = true;
                                                	}
							if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                	{
                                                        	allmovesaremate = false;
                                                	}
							if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        	blackmateinone = true;
                  					nummoves++;
						}
               					board[s.getRowIndex()][x].deletePiece();
               					board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
             				}
             				else
             				{
               					if(board[s.getRowIndex()][x].getColor() == 0)
                				{
				      			Square scopy = board[s.getRowIndex()][x];
                  					board[s.getRowIndex()][x].setPiece('r', 1);
                  					board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					   		DefenseReset();
							if(!blackcheck)
							{
								if(blackmateintwo && !foundamateline && SeeWhiteDeeper(true))
                                                		{
                                                        		foundamateline = true;
                                                		}
								if(allmovesaremate && mateintwo && !SeeWhiteDeeper())
                                                		{
                                                        		allmovesaremate = false;
                                                		}
								if(!blackmateinone && checkfurther && !HasAValidMove())
                                                        		blackmateinone = true;
                     						nummoves++;
							}
							board[s.getRowIndex()][x].setPiece(scopy.getPiece(), 0);
							board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 1);
				   		}
              				break;
             				}
         		}
     		}
	}
	DefenseReset();
	if(blackmateintwo)
	{
		return foundamateline;
	}
	else if(mateintwo)
	{
		return allmovesaremate;
	}
	else if(!checkfurther && nummoves == 0)
	{
		return true;
	}
	else if(checkfurther)
	{
		return blackmateinone;
	}
	return false;
}

bool Chess::HasAValidMove()
{
		std::vector<Square> moveable;
		for(int i = 0; i < 8; i++)
		{
			for(int j = 0; j < 8; j++)
			{
				if(board[i][j].getColor() == 0)
				{
					moveable.push_back(board[i][j]);
				}
			}
		}
		for(unsigned int i = 0; i < moveable.size(); i++)
		{
			if(moveable.at(i).getPiece() == 'p')
			{
						if(moveable.at(i).getRowIndex()+1 < 8 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex(), 'p', 0, ' ', false))
						{
							return true;
					        }
				 		if(moveable.at(i).getRowIndex()+2 < 8 &&board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex(), 'p', 0, ' ', false))
                                        	{
							return true;
						}
						if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex() + 1 < 8 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+1, 'p', 0, ' ', false))
						{
							return true;
						}
                                                if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex() - 1 >= 0 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-1, 'p', 0, ' ', false))
                                                {
							return true;
                                                }
						if(enpassant && board[5][enpassantcol].IsEmpty() && ValidMove(5, enpassantcol, 'p', 0, ' ', false))
						{
							return true;
						}
			}
			if(moveable.at(i).getPiece() == 'n')
			{
						if(moveable.at(i).getRowIndex()+2 < 8 && moveable.at(i).getColumnIndex() + 1 < 8 && board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()+1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', false))
                                        	{
							return true;
                                        	}
						if(moveable.at(i).getRowIndex()+2 < 8 && moveable.at(i).getColumnIndex() - 1 >= 0 && board[moveable.at(i).getRowIndex()+2][moveable.at(i).getColumnIndex()-1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', false))
                                        	{
							return true;
                                        	}
                                                if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex() + 2 < 8 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+2].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex() -2 >= 0 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-2].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()-1 >= 0 && moveable.at(i).getColumnIndex() + 2 < 8 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+2].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+2, 'n', 0, ' ', false) )
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()-1 >= 0 && moveable.at(i).getColumnIndex() - 2 >= 0 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-2].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-2, 'n', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()-2 >= 0 && moveable.at(i).getColumnIndex() + 1 < 8 && board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()+1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()+1, 'n', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()-2 >= 0 && moveable.at(i).getColumnIndex() - 1 >= 0 && board[moveable.at(i).getRowIndex()-2][moveable.at(i).getColumnIndex()-1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()-2, moveable.at(i).getColumnIndex()-1, 'n', 0, ' ', false))
                                                {
							return true;
                                                }
			}
			if(moveable.at(i).getPiece() == 'q')
			{
				for(int x = 1; x <= (7 - moveable.at(i).getRowIndex()); x++)
				{
					if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].IsEmpty())
					{
						if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
						{
							return true;
						}
					}
					else
					{
						if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                        	{
                                                	if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
                                                	{
								return true;
                                                	}
                                        	}

						break;
					}
				}
				for(int x = 1; x <= (7 - moveable.at(i).getColumnIndex()); x++)
				{
					if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].IsEmpty())
					{
						if(ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
						{
							return true;
						}
					}
					else
					{
						if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }

						break;
					}
				}
				for(int x = moveable.at(i).getRowIndex() - 1; x >= 0; x--)
				{
					if(board[x][moveable.at(i).getColumnIndex()].IsEmpty())
					{
						if(ValidMove(x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
						{
							return true;
						}
					}
					else
					{
						if(board[x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                                {
                                                        if(ValidMove(x, moveable.at(i).getColumnIndex(), 'q', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }

						break;
					}
				}
		     		for(int x = moveable.at(i).getColumnIndex() - 1; x >= 0; x--)

				{
					if(board[moveable.at(i).getRowIndex()][x].IsEmpty())
					{
						if(ValidMove(moveable.at(i).getRowIndex(), x, 'q', 0, ' ', false))
						{
							return true;
						}
					}
					else
					{
						if(board[moveable.at(i).getRowIndex()][x].getColor() == 1)
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex(), x, 'q', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }

						break;
					}
				}
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()-x >=0)
                                        {
                                                if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].IsEmpty())
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                	{
                                                        	if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        	{
									return true;
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()+x <8 && moveable.at(i).getColumnIndex()+x <8)
                                        {
                                                if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                	{
                                                        	if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
								{
                                                                        return true;
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()+x < 8)
                                        {
                                                if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                	{
                                                        	if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'q', 0, ' ', false))
                                                        	{
									return true;
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()+x <8 && moveable.at(i).getColumnIndex()-x >=0)
                                        {
                                                if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].IsEmpty())
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                	{
                                                        	if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'q', 0, ' ', false))
                                                        	{
									return true;
                                                        	}
                                                	}

                                                        break;
						}
                                        }
                                }




			}
			if(moveable.at(i).getPiece() == 'k')
			{
						if(moveable.at(i).getRowIndex()+1 < 8 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex(), 'k', 0, ' ', false))
						{
							return true;
						}
                                                if(moveable.at(i).getRowIndex()-1 >= 0 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex(), 'k', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getColumnIndex() + 1 < 8 && board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getColumnIndex() - 1 >= 0 && board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()-1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()-1 >= 0 && moveable.at(i).getColumnIndex() - 1 >= 0 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()-1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', false))
                                                {
                                                       return true;
                                                }
                                                if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex() + 1 < 8 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()+1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()+1 < 8 && moveable.at(i).getColumnIndex() - 1 >= 0 && board[moveable.at(i).getRowIndex()+1][moveable.at(i).getColumnIndex()-1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()+1, moveable.at(i).getColumnIndex()-1, 'k', 0, ' ', false))
                                                {
							return true;
                                                }
                                                if(moveable.at(i).getRowIndex()-1 >= 0 && moveable.at(i).getColumnIndex() + 1 < 8 && board[moveable.at(i).getRowIndex()-1][moveable.at(i).getColumnIndex()+1].getColor() != 0 && ValidMove(moveable.at(i).getRowIndex()-1, moveable.at(i).getColumnIndex()+1, 'k', 0, ' ', false))
                                                {
							return true;
                                                }


			}
			if(moveable.at(i).getPiece() == 'b')
			{
				for(int x = 1; x < 8; x++)
				{
					if(moveable.at(i).getRowIndex()+x < 8 && moveable.at(i).getColumnIndex()+x < 8)
					{
						if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].IsEmpty())
						{
							if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
							{
								return true;
							}
						}
						else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                        {
                                                                if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }


							break;
						}
					}
				}
				for(int x = 1; x < 8; x++)
				{
					if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()-x >=0)
					{
						if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].IsEmpty())
						{
							if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
							{
								return true;
							}
						}
						else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                        {
                                                                if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }

							break;
						}
					}
				}
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()-x >=0 && moveable.at(i).getColumnIndex()+x <8)
                                        {
                                                if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()-x][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                        {
                                                                if(ValidMove(moveable.at(i).getRowIndex()-x, moveable.at(i).getColumnIndex()+x, 'b', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }

                                                        break;
						}
                                        }
                                }
				for(int x = 1; x < 8; x++)
                                {
                                        if(moveable.at(i).getRowIndex()+x <8 && moveable.at(i).getColumnIndex()-x >=0)
                                        {
                                                if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].IsEmpty())
                                                {
                                                        if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
                                                        {
								return true;
                                                        }
                                                }
                                                else
						{
							if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()-x].getColor() == 1)
                                                        {
                                                                if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex()-x, 'b', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }

                                                        break;
						}
                                        }
                                }


			}
			if(moveable.at(i).getPiece() == 'r')
			{
				 for(int x = 1; x <= (7 - moveable.at(i).getRowIndex()); x++)
                                {
                                        if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].IsEmpty())
                                        {
                                                if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                {
							return true;
                                                }
                                        }
					else
					{
						if(board[moveable.at(i).getRowIndex()+x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                                        {
                                                                if(ValidMove(moveable.at(i).getRowIndex()+x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }

						break;
					}
                                }
                                for(int x = 1; x <= (7 - moveable.at(i).getColumnIndex()); x++)
                                {
                                        if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].IsEmpty())
                                        {
                                                if(ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'r', 0, ' ', false))
                                                {
							return true;
                                                }
                                        }
					else
					{
						if(board[moveable.at(i).getRowIndex()][moveable.at(i).getColumnIndex()+x].getColor() == 1)
                                                        {
                                                                if(ValidMove(moveable.at(i).getRowIndex(), moveable.at(i).getColumnIndex()+x, 'r', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }

						break;
					}
                                }
                                for(int x = moveable.at(i).getRowIndex() - 1; x >= 0; x--)
                                {
                                        if(board[x][moveable.at(i).getColumnIndex()].IsEmpty())
                                        {
                                                if(ValidMove(x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                {
							return true;
                                                }
                                        }
					else
					{
						if(board[x][moveable.at(i).getColumnIndex()].getColor() == 1)
                                                        {
                                                                if(ValidMove(x, moveable.at(i).getColumnIndex(), 'r', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }

						break;
					}
                                }
                                for(int x = moveable.at(i).getColumnIndex() - 1; x >= 0; x--)

                                {
                                        if(board[moveable.at(i).getRowIndex()][x].IsEmpty())
                                        {
                                                if(ValidMove(moveable.at(i).getRowIndex(), x, 'r', 0, ' ', false))
                                                {
							return true;
                                                }
                                        }
					else
					{
						if(board[moveable.at(i).getRowIndex()][x].getColor() == 1)
                                                        {
                                                                if(ValidMove(moveable.at(i).getRowIndex(), x, 'r', 0, ' ', false))
                                                                {
									return true;
                                                                }
                                                        }

						break;
					}
                                }

			}
		}
		return false;
	}

bool Chess::SeeWhiteDeeper(bool passback)
{
	bool mate = false;
	bool blackmate = true;
	std::vector<Square> moveable;
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(board[i][j].getColor() == 0)
			{
				moveable.push_back(board[i][j]);
			}
		}
	}
	for(Square s : moveable)
	{
		if(s.getPiece() == 'p')
		{
			if(board[s.getRowIndex()+1][s.getColumnIndex()].IsEmpty())
			{
				if(s.getRowIndex()+1 == 7)
				{
					board[s.getRowIndex()+1][s.getColumnIndex()].setPiece('q', 0);
				}
				else
					board[s.getRowIndex()+1][s.getColumnIndex()].setPiece('p', 0);
				board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
				DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
				{
					blackmate = false;
				}
				else if(!check && DidIWin())
				{
					mate = true;
				}

				board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 0);
				board[s.getRowIndex()+1][s.getColumnIndex()].deletePiece();
				if(!blackmate)
					return false;
			}
			if(s.getRowIndex() == 1 && board[s.getRowIndex()+2][s.getColumnIndex()].IsEmpty() && board[s.getRowIndex()+1][s.getColumnIndex()].IsEmpty())
			{
				board[s.getRowIndex()+2][s.getColumnIndex()].setPiece('p', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }

                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 0);
                                board[s.getRowIndex()+2][s.getColumnIndex()].deletePiece();
				if(!blackmate)
                                        return false;
			}
			if(s.getColumnIndex() + 1 < 8)
			{
				if(board[s.getRowIndex()+1][s.getColumnIndex()+1].getColor() == 1)
				{
					Square scopy = board[s.getRowIndex()+1][s.getColumnIndex()+1];
					if(s.getRowIndex()+1 == 7)
                                	{
                                        	board[s.getRowIndex()+1][s.getColumnIndex()+1].setPiece('q', 0);
                                	}
                                	else
						board[s.getRowIndex()+1][s.getColumnIndex()+1].setPiece('p', 0);
                                	board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                	DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                		{
                                        		mate = true;
                                		}
                                        	board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 0);
                                        	board[s.getRowIndex()+1][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 1);
					if(!blackmate)
                                        	return false;
				}
			}
			if(s.getColumnIndex() - 1 >= 0)
			{
				if(board[s.getRowIndex()+1][s.getColumnIndex()-1].getColor() == 1)
				{
					Square scopy = board[s.getRowIndex()+1][s.getColumnIndex()-1];
					if(s.getRowIndex()+1 == 7)
                                	{
                                        	board[s.getRowIndex()+1][s.getColumnIndex()-1].setPiece('q', 0);
                                	}
                                	else
						board[s.getRowIndex()+1][s.getColumnIndex()-1].setPiece('p', 0);
                                	board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                	DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
                                        	board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 0);
                                        	board[s.getRowIndex()+1][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 1);

					if(!blackmate)
                                        	return false;
				}
			}
			if(s.getRowIndex() == 3  && enpassant && (s.getColumnIndex() - 1 == enpassantcol || s.getColumnIndex() + 1 == enpassantcol))
                        {
                                Square scopy = board[3][enpassantcol];
                                board[2][enpassant].setPiece('p', 0);
                                board[3][enpassantcol].deletePiece();
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[2][enpassant].deletePiece();
                                        board[3][enpassantcol] = scopy;
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('p', 0);
					if(!blackmate)
                                        	return false;
			}

		}
		else if(s.getPiece() == 'k')
		{
			if(s.getRowIndex() + 1 < 8 && board[s.getRowIndex()+1][s.getColumnIndex()].getColor() != 0)
                        {
				Square scopy;
				bool capture = false;
				if(board[s.getRowIndex()+1][s.getColumnIndex()].getColor() == 1)
				{
					capture = true;
					scopy = board[s.getRowIndex()+1][s.getColumnIndex()];
				}
                                board[s.getRowIndex()+1][s.getColumnIndex()].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
					if(capture)
					{
						board[s.getRowIndex()+1][s.getColumnIndex()].setPiece(scopy.getPiece(), 1);
					}
					else
					{
						board[s.getRowIndex()+1][s.getColumnIndex()].deletePiece();
					}
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(!blackmate)
                                        	return false;
                        }
			if(s.getRowIndex() + 1 < 8 && s.getColumnIndex() + 1 < 8 && board[s.getRowIndex()+1][s.getColumnIndex()+1].getColor() != 0)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()+1][s.getColumnIndex()+1].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()+1][s.getColumnIndex()+1];
                                }

                                board[s.getRowIndex()+1][s.getColumnIndex()+1].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(capture)
						board[s.getRowIndex()+1][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 1);
					else
                                        	board[s.getRowIndex()+1][s.getColumnIndex()+1].deletePiece();
					if(!blackmate)
                                        	return false;
                        }
			if(s.getRowIndex() + 1 < 8 && s.getColumnIndex() - 1 >= 0 && board[s.getRowIndex()+1][s.getColumnIndex()-1].getColor() != 0)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()+1][s.getColumnIndex()-1].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()+1][s.getColumnIndex()-1];
                                }
                                board[s.getRowIndex()+1][s.getColumnIndex()-1].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(capture)
						board[s.getRowIndex()+1][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 1);
					else
                                        	board[s.getRowIndex()+1][s.getColumnIndex()-1].deletePiece();
					if(!blackmate)
                                        	return false;
                        }
			if(s.getRowIndex() - 1 >=0  && s.getColumnIndex() - 1 >= 0 && board[s.getRowIndex()-1][s.getColumnIndex()-1].getColor() != 0)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()-1][s.getColumnIndex()-1].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()-1][s.getColumnIndex()-1];
                                }

                                board[s.getRowIndex()-1][s.getColumnIndex()-1].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(capture)
						board[s.getRowIndex()-1][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 1);
					else
                                        	board[s.getRowIndex()-1][s.getColumnIndex()-1].deletePiece();
					if(!blackmate)
                                        	return false;
                        }
			if(s.getRowIndex() - 1 >=0 && s.getColumnIndex() + 1 < 8 && board[s.getRowIndex()-1][s.getColumnIndex()+1].getColor() != 0)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()-1][s.getColumnIndex()+1].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()-1][s.getColumnIndex()+1];
                                }
                                board[s.getRowIndex()-1][s.getColumnIndex()+1].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(capture)
						board[s.getRowIndex()-1][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 1);
					else
                                        	board[s.getRowIndex()-1][s.getColumnIndex()+1].deletePiece();
					if(!blackmate)
                                        	return false;
                        }
			if(s.getColumnIndex() + 1 < 8 && board[s.getRowIndex()][s.getColumnIndex()+1].getColor() != 0)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()][s.getColumnIndex()+1].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()][s.getColumnIndex()+1];
                                }

                                board[s.getRowIndex()][s.getColumnIndex()+1].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(capture)
						board[s.getRowIndex()][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 1);
					else
                                        	board[s.getRowIndex()][s.getColumnIndex()+1].deletePiece();
					if(!blackmate)
                                        	return false;
                        }
			if(s.getRowIndex() - 1 >= 0 && board[s.getRowIndex()-1][s.getColumnIndex()].getColor() != 0)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()-1][s.getColumnIndex()].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()-1][s.getColumnIndex()];
                                }

                                board[s.getRowIndex()-1][s.getColumnIndex()].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(capture)
						board[s.getRowIndex()-1][s.getColumnIndex()].setPiece(scopy.getPiece(), 1);
					else
                                        	board[s.getRowIndex()-1][s.getColumnIndex()].deletePiece();
					if(!blackmate)
                                        	return false;
                        }
			if(s.getColumnIndex() - 1 >= 0 && board[s.getRowIndex()][s.getColumnIndex()-1].getColor() != 0)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()][s.getColumnIndex()-1].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()][s.getColumnIndex()-1];
                                }

                                board[s.getRowIndex()][s.getColumnIndex()-1].setPiece('k', 0);
                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                DefenseReset();
				if(!check && passback && blackmate && !DidIWin(true))
                                {
                                        blackmate = false;
                                }
				else if(!check && DidIWin())
                                {
                                        mate = true;
                                }
                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('k', 0);
					if(capture)
						board[s.getRowIndex()][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 1);
					else
                                        	board[s.getRowIndex()][s.getColumnIndex()-1].deletePiece();
					if(!blackmate)
                                        	return false;
                        }

		}
		else if(s.getPiece() == 'n')
		{
			if(s.getRowIndex()+2 < 8 && s.getColumnIndex()+1 < 8)
                        {
                        	if(board[s.getRowIndex()+2][s.getColumnIndex()+1].getColor() != 0)
                        	{
					Square scopy;
                                	bool capture = false;
                                	if(board[s.getRowIndex()+2][s.getColumnIndex()+1].getColor() == 1)
                                	{
                                        	capture = true;
                                        	scopy = board[s.getRowIndex()+2][s.getColumnIndex()+1];
                                	}

                                	board[s.getRowIndex()+2][s.getColumnIndex()+1].setPiece('n', 0);
                                	board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
					DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
						board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()+2][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 1);
						else
							board[s.getRowIndex()+2][s.getColumnIndex()+1].deletePiece();
						if(!blackmate)
                                        		return false;
                                        
                                }


                        }
			if(s.getRowIndex()+2 < 8 && s.getColumnIndex()-1 >= 0)
                        {
                                if(board[s.getRowIndex()+2][s.getColumnIndex()-1].getColor() != 0)
                                {
					Square scopy;
                                	bool capture = false;
                                	if(board[s.getRowIndex()+2][s.getColumnIndex()-1].getColor() == 1)
                                	{
                                        	capture = true;
                                        	scopy = board[s.getRowIndex()+2][s.getColumnIndex()-1];
                                	}

                                        board[s.getRowIndex()+2][s.getColumnIndex()-1].setPiece('n', 0);
                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                        DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        		mate = true;
                                	}
                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()+2][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 1);
						else
                                                	board[s.getRowIndex()+2][s.getColumnIndex()-1].deletePiece();
						if(!blackmate)
                                                        return false;
                                }


                        }
			if(s.getRowIndex()+1 < 8 && s.getColumnIndex()+2 < 8)
                        {
                                if(board[s.getRowIndex()+1][s.getColumnIndex()+2].getColor() != 0)
                                {
					Square scopy;
                                	bool capture = false;
                                	if(board[s.getRowIndex()+1][s.getColumnIndex()+2].getColor() == 1)
                                	{
                                        	capture = true;
                                        	scopy = board[s.getRowIndex()+1][s.getColumnIndex()+2];
                                	}

                                        board[s.getRowIndex()+1][s.getColumnIndex()+2].setPiece('n', 0);
                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                        DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()+1][s.getColumnIndex()+2].setPiece(scopy.getPiece(), 1);
						else
                                                	board[s.getRowIndex()+1][s.getColumnIndex()+2].deletePiece();
						if(!blackmate)
                                                        return false;
                                }


                        }
			if(s.getRowIndex()+1 < 8 && s.getColumnIndex()-2 >= 0)
                        {
                                if(board[s.getRowIndex()+1][s.getColumnIndex()-2].getColor() != 0)
                                {
					Square scopy;
                                	bool capture = false;
                                	if(board[s.getRowIndex()+1][s.getColumnIndex()-2].getColor() == 1)
                                	{
                                        	capture = true;
                                        	scopy = board[s.getRowIndex()+1][s.getColumnIndex()-2];
                                	}

                                        board[s.getRowIndex()+1][s.getColumnIndex()-2].setPiece('n', 0);
                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                        DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()+1][s.getColumnIndex()-2].setPiece(scopy.getPiece(), 1);
						else
                                                	board[s.getRowIndex()+1][s.getColumnIndex()-2].deletePiece();
						if(!blackmate)
                                                        return false;
                                }


                        }
			if(s.getRowIndex()-1 >= 0 && s.getColumnIndex()+2 < 8)
                        {
				Square scopy;
                                bool capture = false;
                                if(board[s.getRowIndex()-1][s.getColumnIndex()+2].getColor() == 1)
                                {
                                        capture = true;
                                        scopy = board[s.getRowIndex()-1][s.getColumnIndex()+2];
                                }

                                if(board[s.getRowIndex()-1][s.getColumnIndex()+2].getColor() != 0)
                                {
                                        board[s.getRowIndex()-1][s.getColumnIndex()+2].setPiece('n', 0);
                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                        DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()-1][s.getColumnIndex()+2].setPiece(scopy.getPiece(), 1);
						else
                                                	board[s.getRowIndex()-1][s.getColumnIndex()+2].deletePiece();
						if(!blackmate)
                                                        return false;
                                }


                        }
			if(s.getRowIndex()-1 >= 0 && s.getColumnIndex()-2 >= 0)
                        {
                                if(board[s.getRowIndex()-1][s.getColumnIndex()-2].getColor() != 0)
                                {
					Square scopy;
                                	bool capture = false;
                                	if(board[s.getRowIndex()-1][s.getColumnIndex()-2].getColor() == 1)
                                	{
                                        	capture = true;
                                        	scopy = board[s.getRowIndex()-1][s.getColumnIndex()-2];
                                	}

                                        board[s.getRowIndex()-1][s.getColumnIndex()-2].setPiece('n', 0);
                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                        DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()-1][s.getColumnIndex()-2].setPiece(scopy.getPiece(), 1);
						else
                                                	board[s.getRowIndex()-1][s.getColumnIndex()-2].deletePiece();
						if(!blackmate)
                                                        return false;
                                }


                        }
			if(s.getRowIndex()-2 >= 0 && s.getColumnIndex()+1 < 8)
                        {
                                if(board[s.getRowIndex()-2][s.getColumnIndex()+1].getColor() != 0)
                                {
					Square scopy;
                                	bool capture = false;
                                	if(board[s.getRowIndex()-2][s.getColumnIndex()+1].getColor() == 1)
                                	{
                                        	capture = true;
                                        	scopy = board[s.getRowIndex()-2][s.getColumnIndex()+1];
                                	}

                                        board[s.getRowIndex()-2][s.getColumnIndex()+1].setPiece('n', 0);
                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                        DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()-2][s.getColumnIndex()+1].setPiece(scopy.getPiece(), 1);
						else
                                                	board[s.getRowIndex()-2][s.getColumnIndex()+1].deletePiece();
						if(!blackmate)
                                                        return false;
                                }


                        }
			if(s.getRowIndex()-2 >= 0 && s.getColumnIndex()-1 >= 0)
                        {
                                if(board[s.getRowIndex()-2][s.getColumnIndex()-1].getColor() != 0)
                                {
					Square scopy;
                                	bool capture = false;
		                        if(board[s.getRowIndex()-2][s.getColumnIndex()-1].getColor() == 1)
                                	{
                                        	capture = true;
                                        	scopy = board[s.getRowIndex()-2][s.getColumnIndex()-1];
                                	}

                                        board[s.getRowIndex()-2][s.getColumnIndex()-1].setPiece('n', 0);
                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                        DefenseReset();
					if(!check && passback && blackmate && !DidIWin(true))
                                	{
                                        	blackmate = false;
                                	}
					else if(!check && DidIWin())
                                	{
                                        	mate = true;
                                	}
                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('n',0);
						if(capture)
							board[s.getRowIndex()-2][s.getColumnIndex()-1].setPiece(scopy.getPiece(), 1);
						else
                                                	board[s.getRowIndex()-2][s.getColumnIndex()-1].deletePiece();
						if(!blackmate)
                                                        return false;
                                }


                        }

		}
		else if(s.getPiece() == 'q')
		{
			for(int x = 1; x <= (7 - s.getRowIndex()); x++)
                        {
                                        if(board[s.getRowIndex()+x][s.getColumnIndex()].IsEmpty())
                                        {
                                                        
                                                        board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
							DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                			{
                                        			mate = true;
                                			}
                                                        	board[s.getRowIndex()+x][s.getColumnIndex()].deletePiece();
                                                        	board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;

                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()+x][s.getColumnIndex()].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()];
                                                                board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
								DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
									board[s.getRowIndex()+x][s.getColumnIndex()].setPiece(scopy.getPiece(), 1);
									board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;

                                                }

                                                break;
                                        }
                         }
			for(int x = 1; x <= (7 - s.getColumnIndex()); x++)
                        {
                                        if(board[s.getRowIndex()][s.getColumnIndex()+x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()][s.getColumnIndex()+x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;

                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()][s.getColumnIndex()+x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()][s.getColumnIndex()+x];
                                                                board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;
                                                }

                                                break;
                                        }
                         }
			for(int x = s.getRowIndex() - 1; x >= 0; x--)
                        {
                                        if(board[x][s.getColumnIndex()].IsEmpty())
                                        {

                                                        board[x][s.getColumnIndex()].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[x][s.getColumnIndex()].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;

                                        }
                                        else
                                        {
                                                if(board[x][s.getColumnIndex()].getColor() == 1)
                                                {
								Square scopy = board[x][s.getColumnIndex()];
                                                                board[x][s.getColumnIndex()].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[x][s.getColumnIndex()].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;

                                                }

                                                break;
                                        }
                         }
			for(int x = s.getColumnIndex() - 1; x >= 0; x--)
                        {
                                        if(board[s.getRowIndex()][x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()][x].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()][x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;

                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()][x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()][x];
                                                                board[s.getRowIndex()][x].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()][x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;
                                                }

                                                break;
                                        }
                         }
			for(int x = 1; x < 8; x++)
                        {
				if(s.getRowIndex() - x >= 0 && s.getColumnIndex() - x >= 0)
				{
                                        if(board[s.getRowIndex()-x][s.getColumnIndex()-x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()-x][s.getColumnIndex()-x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()-x][s.getColumnIndex()-x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()-x];
                                                                board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;
                                                }

                                                break;
                                        }
				}
                         }
			for(int x = 1; x < 8; x++)
                        {
                                if(s.getRowIndex() - x >= 0 && s.getColumnIndex() + x < 8)
                                {
                                        if(board[s.getRowIndex()-x][s.getColumnIndex()+x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()-x][s.getColumnIndex()+x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;

                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()-x][s.getColumnIndex()+x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()+x];
                                                                board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;

                                                }

                                                break;
                                        }
                                }
                         }
			for(int x = 1; x < 8; x++)
                        {
                                if(s.getRowIndex() + x < 8 && s.getColumnIndex() - x >= 0)
                                {
                                        if(board[s.getRowIndex()+x][s.getColumnIndex()-x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{	
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()+x][s.getColumnIndex()-x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;

                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()+x][s.getColumnIndex()-x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()-x];
                                                                board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;
                                                }

                                                break;
                                        }
                                }
                         }
			for(int x = 1; x < 8; x++)
                        {
                                if(s.getRowIndex() + x < 8 && s.getColumnIndex() + x < 8)
                                {
                                        if(board[s.getRowIndex()+x][s.getColumnIndex()+x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('q', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()+x][s.getColumnIndex()+x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
								if(!blackmate)
                                                        		return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()+x][s.getColumnIndex()+x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()+x];
                                                                board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('q', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('q', 0);
									if(!blackmate)
                                                        			return false;
                                                }

                                                break;
                                        }
                                }
                         }

		}
		else if(s.getPiece() == 'b')
		{
			for(int x = 1; x < 8; x++)
                        {
                                if(s.getRowIndex() + x < 8 && s.getColumnIndex() + x < 8)
                                {
                                        if(board[s.getRowIndex()+x][s.getColumnIndex()+x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('b', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()+x][s.getColumnIndex()+x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
								if(!blackmate)
                                                        		return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()+x][s.getColumnIndex()+x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()+x];
                                                                board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece('b', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()+x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
									if(!blackmate)
                                                                        	return false;
                                                }

                                                break;
                                        }
                                }
                         }
			for(int x = 1; x < 8; x++)
                        {
                                if(s.getRowIndex() + x < 8 && s.getColumnIndex() - x >= 0)
                                {
                                        if(board[s.getRowIndex()+x][s.getColumnIndex()-x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('b', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()+x][s.getColumnIndex()-x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
								if(!blackmate)
                                                                        return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()+x][s.getColumnIndex()-x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()-x];
                                                                board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece('b', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()+x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
									if(!blackmate)
                                                                        	return false;
                                                }

                                                break;
                                        }
                                }
                         }
			for(int x = 1; x < 8; x++)
                        {
                                if(s.getRowIndex() - x >= 0 && s.getColumnIndex() + x < 8)
                                {
                                        if(board[s.getRowIndex()-x][s.getColumnIndex()+x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('b', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()-x][s.getColumnIndex()+x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
								if(!blackmate)
                                                                        return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()-x][s.getColumnIndex()+x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()+x];
                                                                board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece('b', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()-x][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
									if(!blackmate)
                                                                        	return false;
                                                }

                                                break;
                                        }
                                }
                         }
			for(int x = 1; x < 8; x++)
                        {
                                if(s.getRowIndex() - x >= 0 && s.getColumnIndex() - x >= 0)
                                {
                                        if(board[s.getRowIndex()-x][s.getColumnIndex()-x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('b', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()-x][s.getColumnIndex()-x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
								if(!blackmate)
                                                                        return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()-x][s.getColumnIndex()-x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()-x][s.getColumnIndex()-x];
                                                                board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece('b', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()-x][s.getColumnIndex()-x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('b', 0);
									if(!blackmate)
                                                                        	return false;

                                                }

                                                break;
                                        }
                                }
                         }




		}
		else if(s.getPiece() == 'r')
		{
			for(int x = 1; x <= (7 - s.getRowIndex()); x++)
                        {
                                        if(board[s.getRowIndex()+x][s.getColumnIndex()].IsEmpty())
                                        {

                                                        board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('r', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()+x][s.getColumnIndex()].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
								if(!blackmate)
                                                                        return false;

                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()+x][s.getColumnIndex()].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()+x][s.getColumnIndex()];
                                                                board[s.getRowIndex()+x][s.getColumnIndex()].setPiece('r', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()+x][s.getColumnIndex()].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
									if(!blackmate)
                                                                        	return false;

                                                }

                                                break;
                                        }
                         }
			for(int x = 1; x <= (7 - s.getColumnIndex()); x++)
                        {
                                        if(board[s.getRowIndex()][s.getColumnIndex()+x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('r', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()][s.getColumnIndex()+x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
								if(!blackmate)
                                                                        return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()][s.getColumnIndex()+x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()][s.getColumnIndex()+x];
                                                                board[s.getRowIndex()][s.getColumnIndex()+x].setPiece('r', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[s.getRowIndex()][s.getColumnIndex()+x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
									if(!blackmate)
                                                                        	return false;
                                                }

                                                break;
                                        }
                         }
			for(int x = s.getRowIndex() - 1; x >= 0; x--)
                        {
                                        if(board[x][s.getColumnIndex()].IsEmpty())
                                        {

                                                        board[x][s.getColumnIndex()].setPiece('r', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[x][s.getColumnIndex()].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
								if(!blackmate)
                                                                        return false;
                                        }
                                        else
                                        {
                                               if(board[x][s.getColumnIndex()].getColor() == 1)
                                                {
								Square scopy = board[x][s.getColumnIndex()];
                                                                board[x][s.getColumnIndex()].setPiece('r', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
                                                        	{
                                                                	mate = true;
                                                        	}
                                                                        board[x][s.getColumnIndex()].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
									if(!blackmate)
                                                                        	return false;
                                                }

                                                break;
                                        }
                         }
			for(int x = s.getColumnIndex() - 1; x >= 0; x--)
                        {
                                        if(board[s.getRowIndex()][x].IsEmpty())
                                        {

                                                        board[s.getRowIndex()][x].setPiece('r', 0);
                                                        board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                        DefenseReset();
							if(!check && passback && blackmate && !DidIWin(true))
                                			{
                                        			blackmate = false;
                                			}
							else if(!check && DidIWin())
                                                        {
                                                                mate = true;
                                                        }
                                                                board[s.getRowIndex()][x].deletePiece();
                                                                board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
								if(!blackmate)
                                                                        return false;
                                        }
                                        else
                                        {
                                                if(board[s.getRowIndex()][x].getColor() == 1)
                                                {
								Square scopy = board[s.getRowIndex()][x];
                                                                board[s.getRowIndex()][x].setPiece('r', 0);
                                                                board[s.getRowIndex()][s.getColumnIndex()].deletePiece();
                                                                DefenseReset();
								if(!check && passback && blackmate && !DidIWin(true))
                                				{
                                        				blackmate = false;
                                				}
								else if(!check && DidIWin())
        	                                                {
	                                                                mate = true;
                	                                        }

                                                                        board[s.getRowIndex()][x].setPiece(scopy.getPiece(), 1);
                                                                        board[s.getRowIndex()][s.getColumnIndex()].setPiece('r', 0);
									if(!blackmate)
                                                                        	return false;

                                                }

                                                break;
                                        }
                         }




		}
	}
	if(passback)
		return blackmate;
	return mate;

}



int Chess::Decide(double x, std::vector<Move> validmoves)
{
	int bestmoveindex = 0;
	if(validmoves.size() == 0)
	{
		return -1;
	}
	Move bestmove = validmoves.at(0);
	for(unsigned int i = 1; i < validmoves.size(); i++)
	{
		if(validmoves.at(i).getScore() > bestmove.getScore())
		{
			bestmove = validmoves.at(i);
			bestmoveindex = i;
		}
		else if(validmoves.at(i).getScore() == bestmove.getScore())
		{
			if((float)rand()/RAND_MAX < 0.5)
			{
				bestmove = validmoves.at(i);
				bestmoveindex = i;
			}
		}
	}
	if(bestmove.getScore() < -9999999)
	{
		thoughts << "I don't feel so good..." << std::endl;
	}
	if(bestmove.getScore() > 99999999 && bestmove.getScore() < 999999999)
	{
		thoughts << "i'm feeling pretty good about my next move ngl" << std::endl;
	}
	thoughts << std::endl;

	return bestmoveindex;
}
