CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS = -pthread

.PHONY: all clean test
all: server client
server: SRC/server.c SRC/chess.c SRC/chess.h
	$(CC) $(CFLAGS) -o $@ SRC/server.c SRC/chess.c $(LDLIBS)
client: SRC/client.c
	$(CC) $(CFLAGS) -o $@ SRC/client.c
clean:
	rm -f server client tests/test_chess
test: all
	$(CC) $(CFLAGS) -o tests/test_chess tests/test_chess.c SRC/chess.c
	./tests/test_chess
	python3 tests/test_protocol.py
