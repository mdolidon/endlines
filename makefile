CC=cc
CFLAGS= -O3 -Wall -std=c99
LDFLAGS=
BODIES=src/engine.c src/main.c
HEADERS=src/endlines.h
OBJECTS=$(BODIES:.c=.o)
EXECUTABLE=endlines

all: $(BODIES) $(HEADERS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.o:
	$(CC) -c $(CFLAGS) -o $@

clean:
	rm src/*.o src/*~
