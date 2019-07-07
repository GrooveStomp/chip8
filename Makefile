CC			 = /usr/bin/gcc
INC			+= $(shell sdl2-config --cflags)
CFLAGS	+= -g -std=c11 -pedantic -Wall -D_GNU_SOURCE
HEADERS	 = $(wildcard *.h) $(wildcard external/*.h)
LIBS		+= $(shell sdl2-config --libs) -lSDL2main -lGL -lGLEW -lm -lpthread -lsoundio

SRC			 = input.c main.c opcode.c sound.c system.c ui.c
OBJFILES = $(patsubst %.c,%.o,$(SRC))
OBJ			 = $(addprefix build/, $(OBJFILES))
LINTFILES= $(patsubst %.c,__%.c,$(SRC)) $(patsubst %.c,_%.c,$(SRC))
BIN			 = build/chip8

DEFAULT_GOAL := $(BIN)
.PHONY: clean valgrind splint uno

build/%.o: $(HEADERS) %.c
	mkdir -p $(@D)
	$(CC) -c $*.c $(INC) $(CFLAGS) -o $@

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)

clean:
	rm -f build/* || true
	rm -f core || true
	rm -rf ${LINTFILES} || true

valgrind:
	echo "valgrind"

splint:
	splint ${INC} ${CFLAGS} ${SRC}

uno:
	uno ${INC} ${SRC}
