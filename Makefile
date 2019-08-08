#******************************************************************************
# File: Makefile
# Created: 2019-06-27
# Updated: 2019-08-04
# Author: Aaron Oman
# Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
#******************************************************************************
CC       = /usr/bin/gcc
INC     += $(shell sdl2-config --cflags)
HEADERS  = $(wildcard *.h) $(wildcard external/*.h)
LIBS    += $(shell sdl2-config --libs) -lSDL2main -lGL -lGLEW -lm -lpthread -lsoundio
CFLAGS  += -std=c11 -pedantic -Wall -D_GNU_SOURCE

SRC_DEP  = gfxinputthread.c threadsync.c timerthread.c
SRC      = input.c main.c opcode.c sound.c system.c ui.c graphics.c
OBJFILES = $(patsubst %.c,%.o,$(SRC))
LINTFILES= $(patsubst %.c,__%.c,$(SRC)) $(patsubst %.c,_%.c,$(SRC))

RELDIR = release
RELOBJ = $(addprefix $(RELDIR)/,$(OBJFILES))
RELEXE = $(RELDIR)/chip8
RELFLG = -O3

DBGDIR = debug
DBGOBJ = $(addprefix $(DBGDIR)/,$(OBJFILES))
DBGEXE = $(DBGDIR)/chip8
DBGFLG = -g -Og

TSTDIR = test
TSTSRC = $(wildcard $(TSTDIR)/*.c)
TSTEXE = $(patsubst $(TSTDIR)/%.c,$(TSTDIR)/%,$(TSTSRC))
TSTLIB = $(LIBS) -ldl
TSTOBJ = $(filter-out $(TSTDIR)/main.o,$(addprefix $(TSTDIR)/,$(OBJFILES)))

DEFAULT_GOAL := $(release)
.PHONY: clean debug docs release splint test uno valgrind

release: $(RELEXE)

$(RELEXE): $(RELOBJ)
	$(CC) -o $@ $^ $(LIBS)

$(RELDIR)/%.o: %.c $(HEADERS) $(SRC_DEP)
	@mkdir -p $(@D)
	$(CC) -c $*.c $(INC) $(CFLAGS) $(RELFLG) -o $@

debug: $(DBGEXE)

$(DBGEXE): $(DBGOBJ)
	$(CC) -o $@ $^ $(LIBS)

$(DBGDIR)/%.o: %.c $(HEADERS) $(SRC_DEP)
	@mkdir -p $(@D)
	$(CC) -c $*.c $(INC) $(CFLAGS) $(DBGFLG) -o $@

test: $(TSTEXE)

$(TSTDIR)/%_test: $(TSTOBJ) $(TSTDIR)/%_test.o
	$(CC) -o $@ $(TSTDIR)/$*_test.o $(filter-out $(TSTDIR)/$*.o,$(TSTOBJ)) $(TSTLIB)

$(TSTDIR)/%_test.o: $(HEADERS) $(TSTDIR)/gstest.h $(TSTDIR)/%_test.c
	$(CC) -c $(TSTDIR)/$*_test.c $(INC) $(CFLAGS) $(DBGFLG) -o $@

$(TSTDIR)/%.o: %.c $(HEADERS)
	$(CC) -c $*.c $(INC) $(CFLAGS) $(DBGFLG) -o $@

runtests: test
	$(foreach exe,$(TSTEXE),./$(exe);)

clean:
	rm -rf core debug release ${LINTFILES} ${DBGOBJ} ${RELOBJ} ${TSTOBJ} ${TSTEXE} cachegrind.out.* callgrind.out.*

docs:
	doxygen .doxygen.conf

valgrind:
	echo "valgrind"

splint:
	splint ${INC} ${CFLAGS} ${SRC}

uno:
	uno ${INC} ${SRC}
