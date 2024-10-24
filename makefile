all: build

BIN=bin
CXX=clang++
CXXFLAGS=-std=c++17 -glldb -fstandalone-debug -Wall
SRC_FILES=passa_tempo.cpp main.cpp
INPUT_FILE=input.txt

run: build
	./$(BIN)

build: passa_tempo.hpp $(SRC_FILES)
	[[ -z "${DEV}" ]] && g++ $(SRC_FILES) -o $(BIN) || $(CXX) $(CXXFLAGS) $(SRC_FILES) -o $(BIN)

clean:
	rm $(BIN) 

test: build
	./$(BIN) < $(INPUT_FILE)

.PHONY: all run build clean test
