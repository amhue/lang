all: main.o lexer.o parser.o
	g++ -g main.o lexer.o parser.o -o main

main.o: ./src/main.cpp ./src/lexer.hpp
	g++ -g -Wall -Wextra -c ./src/main.cpp -o main.o

lexer.o: ./src/lexer.cpp
	g++ -g -Wall -Wextra -c ./src/lexer.cpp -o lexer.o

parser.o: ./src/parser.cpp
	g++ -g -Wall -Wextra -c ./src/parser.cpp -o parser.o
