#Makefile du client JolChap v0.1

CC=gcc
CFLAGS= -g -Wall
SRCS=client.c
OBJS=$(SRCS:.c=.o)
MAIN=client

.PHONY: depend clean

all: $(MAIN)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

clean:
	$(RM) *.o *~ $(MAIN)
