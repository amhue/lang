CFLAGS = -Wall -Wextra -g -O2
OBJECTS = ./out/main.o ./out/lexer.o ./out/parser.o ./out/semantic.o

all: out $(OBJECTS)
	g++ $(OBJECTS) -o ./out/main

out:
	mkdir -p ./out

./out/main.o: ./src/main.cpp
	g++ -c ./src/main.cpp -o ./out/main.o

./out/lexer.o: ./src/lexer.cpp
	g++ -c ./src/lexer.cpp -o ./out/lexer.o

./out/parser.o: ./src/parser.cpp
	g++ -c ./src/parser.cpp -o ./out/parser.o

./out/semantic.o: ./src/semantic.cpp
	g++ -c ./src/semantic.cpp -o ./out/semantic.o

clean:
	rm -rf ./out

.PHONY: clean all out
