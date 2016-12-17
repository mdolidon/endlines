M4=m4
CFLAGS= -O2 -Wall -std=c99
BODIES=src/file_operations.c src/walkers.c src/convert_stream.c src/main.c src/utils.c
HEADERS=src/endlines.h src/walkers.h src/known_binary_extensions.h
OBJECTS=$(BODIES:.c=.o)
LDFLAGS=-lrt

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
	(cd test; bash runtest.sh)
