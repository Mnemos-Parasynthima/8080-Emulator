CC = gcc
CFLAGS = -Wall
INCLUDES = -Iheaders -Iheaders/base/ -Iheaders/kernel/ -Iheaders/stages/

SRCS = base/machine.c base/hardware.c base/mem.c kernel/aef-loadrun.c stages/fetch.c main.c Error.c

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