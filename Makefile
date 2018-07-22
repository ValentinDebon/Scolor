CC=clang
CFLAGS=-pedantic -Wall -fPIC
SOURCES=$(wildcard src/*.c)
LIBRARIES=-lSDL2 -lSDL2_ttf
SCOLOR=scolor

.PHONY: all clean

all: $(SCOLOR)

clean:
	rm -rf $(SCOLOR)

$(SCOLOR): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARIES)
