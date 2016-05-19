M4=m4
CFLAGS= -O3 -Wall -std=c99
BODIES=src/walkers.c src/engine.c src/main.c
HEADERS=src/endlines.h src/walkers.h src/known_binary_extensions.h
OBJECTS=$(BODIES:.c=.o)

all: $(BODIES) $(HEADERS) endlines

endlines: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

src/engine.c: src/engine.c.m4
	$(M4) src/engine.c.m4 > src/engine.c

clean:
	rm src/*.o src/engine.c endlines

install: endlines
	mv endlines /usr/local/bin/endlines

test: endlines
	(cd test; ./runtest.sh)
