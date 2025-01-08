CC = gcc
CFLAGS = -Wall
INCLUDES = -I../headers/assembler -I../headers/

SRCS = assembler.c preprocessor.c lexer-parser.c translator-generator.c SymbolTable.c utils.c ../Error.c

OBJS = $(SRCS:%.c=%.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

all: as

as: $(OBJS)
	$(CC) $(CFLAGS) -o as $(OBJS)

debug: CFLAGS += -g -O0
debug: as

clean:
	rm *.o
	rm as