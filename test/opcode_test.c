#include <stdio.h>

#include "gstest.h"

#include "../opcode.h"
#include "../opcode.c"

int GSTestNumTestsRun = 0;
char GSTestErrMsg[256];

static char *FirstTest() {
        int bar = 1;
        GSTestAssert(bar == 2, "Expected bar(%d) to equal(%d)", bar, 2);
        return NULL;
}

static char *RunAllTests() {
        GSRunTest(FirstTest);
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
