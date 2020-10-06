PREFIX?=/usr/local

BODIES=$(wildcard src/*.c)
OBJECTS=$(BODIES:.c=.o)

CFLAGS=-O2 -Wall -std=c99
LDFLAGS=

.PHONY: test install uninstall clean


endlines: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: endlines
	@(cd test; bash runtest.sh)

install: endlines man/endlines.1
	install -D -m755 endlines $(DESTDIR)$(PREFIX)/bin/endlines
	install -D -m755 man/endlines.1 $(DESTDIR)$(PREFIX)/man/man1/endlines.1
	gzip $(DESTDIR)$(PREFIX)/man/man1/endlines.1
	@echo
	@echo "Install finished."
	@echo "The binary is located at $(DESTDIR)$(PREFIX)/bin/endlines"
	@echo "The man pages is located at $(DESTDIR)$(PREFIX)/man/man1/endlines.1.gz"

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/endlines $(DESTDIR)$(PREFIX)/man/man1/endlines.1.gz
	@echo
	@echo "Uninstall finished."

clean:
	-rm src/*.o endlines


# Dependencies on headers
src/command_line_parser.o: src/command_line_parser.h
src/convert_stream.o: src/endlines.h
src/file_operations.o: src/endlines.h
src/file_operations.o: src/walkers.h
src/main.o: src/command_line_parser.h
src/main.o: src/endlines.h
src/main.o: src/walkers.h
src/utils.o: src/endlines.h
src/utils.o: src/known_binary_extensions.h
src/walkers.o: src/walkers.h
