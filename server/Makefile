CC=gcc
CFLAGS= -g -Wall
SRCS=server.c command.c list.c
OBJS=$(SRCS:.c=.o)
MAIN=server

.PHONY: depend clean

all: $(MAIN)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

clean:
	$(RM) *.o *~ $(MAIN)
