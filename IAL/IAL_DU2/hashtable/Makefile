CC=gcc
CFLAGS=-Wall -std=c11 -pedantic
FILES=hashtable.c test.c test_util.c

.PHONY: test clean

test: $(FILES)
	rm -f test
	$(CC) $(CFLAGS) -o $@ $(FILES)
	./test

clean:
	rm -f test
