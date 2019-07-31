/******************************************************************************
  File: opcode_test.c
  Created: 2019-07-07
  Updated: 2019-07-23
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdio.h>

#include "gstest.h"

#include "../opcode.h"
#include "../opcode.c"

// Disables warnings on some sloppy string formatting usage. ie., function
// pointer instead of void pointer.
#pragma GCC diagnostic ignored "-Wformat="

int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

char *TestOpcodeInstruction() {
        struct opcode *c = OpcodeInit();

        unsigned short expected = 0xF065;
        c->instruction = expected;

        unsigned short actual = OpcodeInstruction(c);
        GSTestAssert(expected == actual, "Expected opcode to be 0x%04X, but was 0x%02X", expected, actual);

        OpcodeDeinit(c);
        return NULL;
}

char *TestOpcodeDescription() {
        struct opcode *c = OpcodeInit();

        // c->debug_fn_map[34] = (struct opcode_fn_map){ "FX65", FnFX65, "Fill V0 through VX with values from memory starting at I" };
        char *expected = "FX65: Fill V0 through VX with values from memory starting at I";
        unsigned short opcode = 0xF065;
        c->instruction = opcode;
        OpcodeDecode(c);

        char actual[256];
        OpcodeDescription(c, actual, 256);

        int result = strncmp(actual, expected, strlen(expected));
        GSTestAssert(result == 0, "Expected \"%s\" to be \"%s\"", actual, expected);

        OpcodeDeinit(c);
        return NULL;
}

char *TestHighByte() {
        struct opcode *c = OpcodeInit();
        c->instruction = 0xAABB;
        unsigned char expected = 0xAA;
        unsigned char actual = HighByte(c);

        GSTestAssert(actual == expected, "Expected 0x%02X to be 0x%02X", actual, expected);

        OpcodeDeinit(c);
        return NULL;
}

char *TestLowByte() {
        struct opcode *c = OpcodeInit();
        c->instruction = 0xAABB;
        unsigned char expected = 0xBB;
        unsigned char actual = LowByte(c);

        GSTestAssert(actual == expected, "Expected 0x%02X to be 0x%02X", actual, expected);

        OpcodeDeinit(c);
        return NULL;
}

char *TestNibbleAt() {
        struct opcode *c = OpcodeInit();
        c->instruction = 0xFEDC;

        GSTestAssert(NibbleAt(c, 0) == 0xC, "Expected C");
        GSTestAssert(NibbleAt(c, 1) == 0xD, "Expected D");
        GSTestAssert(NibbleAt(c, 2) == 0xE, "Expected E");
        GSTestAssert(NibbleAt(c, 3) == 0xF, "Expected F");

        OpcodeDeinit(c);
        return NULL;
}

char *TestOpcodeInit() {
        struct opcode *c = OpcodeInit();
        GSTestAssert(NULL != c, "Expected opcode not to be null");
        OpcodeDeinit(c);
        return NULL;
}

char *TestOpcodeFetch() {
        struct system *s = SystemInit(0);
        struct opcode *c = OpcodeInit();

        s->memory[s->pc] = 0xF0;
        s->memory[s->pc+1] = 0x65;

        OpcodeFetch(c, s);
        GSTestAssert(0xF065 == c->instruction, "Expected 0x%04X to equal 0xF065", c->instruction);

        OpcodeDeinit(c);
        SystemDeinit(s);
        return NULL;
}

char *TestOpcodeDecode() {
        struct opcode *c = OpcodeInit();

        c->instruction = 0x00E0;
        OpcodeDecode(c);
        GSTestAssert(c->fn == Fn00E0, "Expected c->fn(%p) to be Fn00E0(%p)", c->fn, Fn00E0);

        c->instruction = 0x7010;
        OpcodeDecode(c);
        GSTestAssert(c->fn == Fn7XNN, "Expected c->fn(%p) to be Fn7XNN(%p)", c->fn, Fn7XNN);


        c->instruction = 0xF018;
        OpcodeDecode(c);
        GSTestAssert(c->fn == FnFX18, "Expected c->fn(%p) to be FnFX18(%p)", c->fn, FnFX18);

        OpcodeDeinit(c);
        return NULL;
}

char *TestOpcodeExecute() {
        // TODO Implement me.
        return NULL;
}

static char *RunAllTests() {
        GSTestRun(TestOpcodeInstruction);
        GSTestRun(TestOpcodeDescription);
        GSTestRun(TestHighByte);
        GSTestRun(TestLowByte);
        GSTestRun(TestNibbleAt);
        GSTestRun(TestOpcodeInit);
        GSTestRun(TestOpcodeFetch);
        GSTestRun(TestOpcodeDecode);
        GSTestRun(TestOpcodeExecute);
        return NULL;
}

int main(int argC, char **argV) {
        char *result = RunAllTests();
        if (result != NULL) {
                printf("%s\n", result);
        } else {
                printf("ALL TESTS PASSED\n");
        }
        printf("opcode_test tests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
