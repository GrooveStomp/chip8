/******************************************************************************
  File: system_test.c
  Created: 2019-07-07
  Updated: 2019-07-23
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdio.h>

#include "gstest.h"

#include "../system.h"
#include "../system.c"

int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

static char *FirstTest() {
        int bar = 1;
        GSTestAssert(bar == 2, "Expected bar(%d) to equal(%d)", bar, 2);
        return NULL;
}

static char *RunAllTests() {
        GSTestRun(FirstTest);
        return NULL;
}

int main(int argC, char **argV) {
        char *result = RunAllTests();
        if (result != NULL) {
                printf("%s\n", result);
        } else {
                printf("ALL TESTS PASSED\n");
        }
        printf("system_test tests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
