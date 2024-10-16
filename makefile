all: build

BIN=bin
CXX=clang++
CXXFLAGS=-std=c++17 -glldb -fstandalone-debug -Wall
SRC_FILES=passa_tempo.cpp main.cpp

run: build
	./$(BIN)

build: passa_tempo.hpp $(SRC_FILES)
	[[ -z "${DEV}" ]] && g++ $(SRC_FILES) -o $(BIN) || $(CXX) $(CXXFLAGS) $(SRC_FILES) -o $(BIN)

clean:
	rm $(BIN) 

.PHONY: all run build clean
