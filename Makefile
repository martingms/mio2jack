CC=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra -Werror -pedantic
LDFLAGS=-ljack -lsndio

.PHOHY: all clean

all: mio2jack

mio2jack: mio2jack.c
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f mio2jack
