CC=gcc-4.9
CFLAGS= -m64 -std=c11 -Wall -Werror -pedantic -ggdb
BIN=./bin/
SOURCE=./src/
.PHONY: clean
.PHONY: test

test: all
	./run_tests.sh

LIST= $(BIN)paola.o $(BIN)gen.o $(BIN)lexer.o $(BIN)parser.o $(BIN)semcheck.o $(BIN)list.o $(BIN)symtable.o $(BIN)errors.o \
      $(BIN)utils.o

all: $(LIST)
	$(CC) $(CFLAGS) -o $(BIN)paola $(LIST)

$(BIN)%.o: $(SOURCE)%.c $(SOURCE)%.h
	$(CC) $(CFLAGS) $(SOURCE)$*.c -c -o $(BIN)$*.o

$(BIN)%.o: $(SOURCE)%.c
	$(CC) $(CFLAGS) $(SOURCE)$*.c -c -o $(BIN)$*.o

clean:
	rm -rf $(BIN)*.o
