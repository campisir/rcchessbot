CXX = g++
CXXFLAGS = -g -std=c++11 -Wall

main: move.o square.o chess.o main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) main *o
