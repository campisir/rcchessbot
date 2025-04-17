CXX = emcc
CXXFLAGS = -g -std=c++11 -Wall

main: move.o square.o chess.o main.cpp
	    $(CXX) $(CXXFLAGS) -o main.js $^ -s EXPORTED_FUNCTIONS='["_my_main"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap", "ccall"]'

%.o: %.cpp
	    $(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	    $(RM) main.js main.wasm *.o
