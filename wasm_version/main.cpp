#include "chess.hpp"

extern "C" {
const char* my_main(const char* moves)
{
	std::string moves_str(moves);
	Chess chess;
	std::string result = chess.MoveFromPos(moves_str);
	char* output = new char[result.length() + 1];
    std::strcpy(output, result.c_str());
    return output;
}
}

