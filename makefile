CC=cc
CFLAGS= -O3 -Wall -std=c99
LDFLAGS=
BODIES=src/engine.c src/main.c
HEADERS=src/endlines.h
OBJECTS=$(BODIES:.c=.o)

all: $(BODIES) $(HEADERS) endlines

endlines: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.o:
	$(CC) -c $(CFLAGS) -o $@

clean:
	rm src/*.o endlines

install: endlines
	mv endlines /usr/local/bin/endlines

test: endlines
	(cd test; ./runtest.sh)
