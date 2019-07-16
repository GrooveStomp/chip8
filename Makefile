#******************************************************************************
# File: Makefile
# Created: (No later than 2019-07-07)
# Updated: 2019-07-14
# Author: Aaron Oman
# Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
#******************************************************************************
CC			 = /usr/bin/gcc
INC			+= $(shell sdl2-config --cflags)
CFLAGS	+= -g -std=c11 -pedantic -Wall -D_GNU_SOURCE
HEADERS	 = $(wildcard *.h) $(wildcard external/*.h)
LIBS		+= $(shell sdl2-config --libs) -lSDL2main -lGL -lGLEW -lm -lpthread -lsoundio

SRC			 = input.c main.c opcode.c sound.c system.c ui.c graphics.c
OBJFILES = $(patsubst %.c,%.o,$(SRC))
OBJ			 = $(addprefix build/, $(OBJFILES))
LINTFILES= $(patsubst %.c,__%.c,$(SRC)) $(patsubst %.c,_%.c,$(SRC))

BINDIR   = bin
BIN			 = $(BINDIR)/chip8

TEST_SRC = $(wildcard test/*.c)
TEST_BIN = $(patsubst test/%.c,bin/%,$(TEST_SRC))
TEST_OBJ = $(filter-out build/main.o,$(OBJ))

DEFAULT_GOAL := $(BIN)
.PHONY: clean valgrind splint uno test $(BINDIR)

bin/%_test: $(TEST_OBJ) build/%_test.o | $(BINDIR)
	$(CC) -o $@ build/$*_test.o $(filter-out build/$*.o,$(TEST_OBJ)) $(LIBS)

build/%_test.o: $(HEADERS) test/gstest.h test/%_test.c
	$(CC) -c test/$*_test.c $(INC) $(CFLAGS) -o $@

build/%.o: $(HEADERS) %.c
	mkdir -p $(@D)
	$(CC) -c $*.c $(INC) $(CFLAGS) -o $@

$(BIN): $(OBJ) | $(BINDIR)
	$(CC) -o $@ $(OBJ) $(LIBS)

$(BINDIR):
	mkdir -p $(BINDIR)

tests: $(TEST_BIN) | $(BINDIR)

runtests: tests
	$(foreach bin,$(TEST_BIN),./$(bin);)

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
