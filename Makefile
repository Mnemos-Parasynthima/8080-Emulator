CC = gcc
CFLAGS = -Wall
INCLUDES = -Iheaders -Iheaders/base/ -Iheaders/kernel/ -Iheaders/pipe/

SRCS = base/machine.c kernel/aef-loadrun.c main.c Error.c

OBJS = $(SRCS:%.c=%.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

all: emu

emu: $(OBJS)
	$(CC) $(CFLAGS) -o emu $(OBJS)

debug: CFLAGS += -g -O0
debug: emu

clean:
	rm *.o
	rm emu