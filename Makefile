CC=clang
CFLAGS=-pedantic -Wall -fPIC
SOURCES=$(wildcard src/*.c)
ifeq ($(shell uname),Darwin)
LIBRARIES=-framework SDL2 -framework SDL2_ttf
else
LIBRARIES=-lSDL2 -lSDL2_ttf
endif
SCOLOR=scolor

.PHONY: all clean

all: $(SCOLOR)

clean:
	rm -rf $(SCOLOR)

$(SCOLOR): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARIES)
