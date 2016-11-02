M4=m4
CFLAGS= -O3 -Wall -std=c99
BODIES=src/walkers.c src/engine.c src/main.c
HEADERS=src/endlines.h src/walkers.h src/known_binary_extensions.h
OBJECTS=$(BODIES:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(BODIES) $(HEADERS) endlines

endlines: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

clean:
	rm src/*.o endlines

install: endlines
	mv endlines /usr/local/bin/endlines

test: endlines
	(cd test; ./runtest.sh)
