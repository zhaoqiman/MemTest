# Makefile for time_count

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS =
INCLUDE = -I../
SRCS = arrary_test.c ../mem_test.c
OBJS = $(SRCS:.c=.o)
EXEC = arrary_test

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJS)
