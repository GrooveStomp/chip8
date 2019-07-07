#include <stdio.h>

#include "gstest.h"

#include "../system.h"
#include "../system.c"

int GSTestNumTestsRun = 0;
char GSTestErrMsg[256];

char errMsg[256];

static char *FirstTest() {
        int bar = 1;
        snprintf(errMsg, 256, "Expected bar(%d) to equal(%d)", bar, 2);
        GSTestAssert(bar == 2, errMsg);
        return NULL;
}

static char *RunAllTests() {
        GSRunTest(FirstTest);
        return NULL;
}

int main(int argC, char **argV) {
        char *result = RunAllTests();
        if (result != 0) {
                printf("%s\n", result);
        } else {
                printf("ALL TESTS PASSED\n");
        }
        printf("system_test tests run: %d\n", GSTestNumTestsRun);

        return result != 0;
}
