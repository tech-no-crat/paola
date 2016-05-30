CC=g++
.PHONY: clean

all: awec.o gen.o lexer.o parser.o semcheck.o list.o symtable.o errors.o
	$(CC) -o awec awec.o gen.o lexer.o parser.o semcheck.o list.o symtable.o errors.o

awec.o: awec.c

lexer.o: lexer.c lexer.h
parser.o: parser.c parser.h
semcheck.o: semcheck.c semcheck.h
gen.o: gen.c gen.h
list.o: list.c list.h
symtable.o: symtable.c symtable.h
errors.o: errors.c errors.h

clean:
	rm -rf *.o
