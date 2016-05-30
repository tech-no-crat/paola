CC=gcc
CFLAGS= -m64 -std=c99
BIN=./bin/
SOURCE=./src/
.PHONY: clean
.PHONY: test

test: all
	./run_tests.sh

LIST= $(BIN)awec.o $(BIN)gen.o $(BIN)lexer.o $(BIN)parser.o $(BIN)semcheck.o $(BIN)list.o $(BIN)symtable.o $(BIN)errors.o
all: $(LIST)
	$(CC) $(CFLAGS) -o $(BIN)awec $(LIST)

$(BIN)%.o: $(SOURCE)%.c $(SOURCE)%.h
	$(CC) $(CFLAGS) $(SOURCE)$*.c -c -o $(BIN)$*.o

$(BIN)%.o: $(SOURCE)%.c
	$(CC) $(CFLAGS) $(SOURCE)$*.c -c -o $(BIN)$*.o

clean:
	rm -rf $(BIN)*.o
