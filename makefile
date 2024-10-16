all: build

BIN=bin
# TODO CHANGE ME TO g++
CXX=clang++
# TODO CHANGE THESE TO g++ FLAGS
CXXFLAGS=-std=c++17 -glldb -fstandalone-debug -Wall

run: build
	./$(BIN)

build: passa_tempo.cpp passa_tempo.hpp main.cpp
	$(CXX) $(CXXFLAGS) passa_tempo.cpp main.cpp -o $(BIN)

clean:
	rm $(BIN) 

.PHONY: all run build clean
