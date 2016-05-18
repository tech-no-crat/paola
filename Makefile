CC=g++
.PHONY: clean

all: awec.o gen.o lexer.o parser.o list.o
	$(CC) -o awec awec.o gen.o lexer.o parser.o list.o

awec.o: awec.c

lexer.o: lexer.c lexer.h
parser.o: parser.c parser.h
gen.o: gen.c gen.h
list.o: list.c list.h

clean:
	rm -rf *.o
